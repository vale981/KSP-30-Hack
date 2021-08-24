/*****************************************************************************/
/*                                    Pins                                   */
/*****************************************************************************/

#include "./pro_micro.h"

/*****************************************************************************/
/*                                Convenience                                */
/*****************************************************************************/
constexpr int col_pins[] = {KD1, KD2, KD3, KD4, KD5, KD6, KD7, KD8};
constexpr int num_col_pins = sizeof(col_pins) / sizeof(col_pins[0]);
constexpr int num_half_pins = num_col_pins / 2;

// first hit
constexpr int first_col[] = {KD1, KD2, KD3, KD4};

// second hit
constexpr int second_col[] = {KD5, KD6, KD7, KD8};

constexpr int selector_pins[] = {SA1, SA2, SA3, SA4, SA5};
constexpr int num_selector_pins = sizeof(selector_pins) / sizeof(selector_pins[0]);

enum Range { LO = 0, MID, HI };
constexpr Range ranges[] = {Range::LO, Range::MID, Range::HI};

// rows in range
constexpr int rows[] = {8, 8, 6};
constexpr int range_offsets[] = {0, rows[1] * num_half_pins,
                                 rows[0] * num_half_pins +
                                     rows[1] * num_half_pins};

/*****************************************************************************/
/*                                 Keys/Notes                                */
/*****************************************************************************/
constexpr int midi_base = 21;
constexpr int num_keys = 88;

/*****************************************************************************/
/*                                  Velocity                                 */
/*****************************************************************************/
constexpr double min_velocity = 0.00005;
constexpr double max_velocity = 0.00030;

/*****************************************************************************/
/*                                Keyscanning                                */
/*****************************************************************************/

void select_range(Range range) {
  switch (range) {
  case Range::LO:
    digitalWrite(RS1, LOW);
    digitalWrite(RS2, LOW);
    break;

  case Range::MID:
    digitalWrite(RS1, HIGH);
    digitalWrite(RS2, LOW);
    break;

  case Range::HI:
    digitalWrite(RS1, LOW);
    digitalWrite(RS2, HIGH);
    break;
  }
}

void select_row(int row) {
  digitalWrite(SEL_A, row & 1);

  row >>= 1;
  digitalWrite(SEL_B, row & 1);

  row >>= 1;
  digitalWrite(SEL_C, row & 1);
}

// UNUSED
/* int scan_row(bool second) { */
/*   const int (&pins)[num_half_pins] = second ? second_col : first_col; */

/*   for(int i= 0; i < num_half_pins; i++) { */
/*     if (!digitalRead(pins[i])) */
/*       return i; */
/*   } */

/*   return -1; */
/* } */

int get_midi_note(Range range, int row, int col) {
  return midi_base + row * num_half_pins + col + range_offsets[range];
}

char midi_velocity(double velocity) {
  return (constrain(velocity, min_velocity, max_velocity) - min_velocity) /
         max_velocity * 127;
}

/*****************************************************************************/
/*                                   State                                   */
/*****************************************************************************/
unsigned long S1[num_keys]; // timer value for first hit or 0
bool S2[num_keys];          // whether key is pressed
bool SSTE_PRESSED = false;

void setup() {
  /***************************************************************************/
  /*                                  Serial                                 */
  /***************************************************************************/
  Serial.begin(9600);

  /***************************************************************************/
  /*                                   Pins                                  */
  /***************************************************************************/
  for (int i = 0; i < num_col_pins; i++) {
    pinMode(col_pins[i], INPUT_PULLUP);
  }

  for (int i = 0; i < num_selector_pins; i++) {
    pinMode(selector_pins[i], OUTPUT);
  }

  for (int i = 0; i < num_keys; i++) {
    S2[i] = false;
  }

  // Pedal
  pinMode(SSTE, INPUT_PULLUP);

  digitalWrite(RS1, LOW);
  digitalWrite(RS2, LOW);
  digitalWrite(SEL_A, LOW);
  digitalWrite(SEL_B, LOW);
  digitalWrite(SEL_C, LOW);
}

void send_note_down(int key, double velocity) {
  Serial.print("Key DOWN: ");
  Serial.print(key);
  Serial.print(" ");
  Serial.println(midi_velocity(velocity), DEC);
}

void send_note_up(int key) {
  Serial.print("Key UP: ");
  Serial.println(key);
}

void send_pedal(bool down) {
  if (down)
    Serial.println("SSTE DOWN");
  else
    Serial.println("SSTE UP");
}

void loop() {
  int key = 0;

  // keys
  for (const auto range : ranges) {
    select_range(range);

    for (int row = 0; row < rows[range]; row++) {
      select_row(row);

      // read rows
      for (int i = 0; i < num_half_pins; i++) {
        const bool s1_was_triggered = !(S1[key] == 0);
        if (!digitalRead(first_col[i])) { // key pressed
          if (!s1_was_triggered) {        // has not been depreesed before
            S1[key] = micros();
          }

          // look for second fire
          const bool s2_was_triggered = S2[key];
          if (!digitalRead(second_col[i])) { // stage 2 pressed
            if (!s2_was_triggered) {
              S2[key] = true;

              auto velocity = 1 / static_cast<double>(micros() - S1[key]);
              send_note_down(key, velocity);
            }
          } else if (s2_was_triggered) { // stage 2 released
            S2[key] = false;
            send_note_up(key);
          }
        } else if (s1_was_triggered) { // s1 released
          S1[key] = 0;

          if (S2[key]) {
            S2[key] = false;
            send_note_up(key);
          }
        }

        key++;
      }
    }
  }

  // pedals
  if (digitalRead(SSTE) != SSTE_PRESSED) {
    SSTE_PRESSED = !SSTE_PRESSED;
    send_pedal(SSTE_PRESSED);
  }
}
