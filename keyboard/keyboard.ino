/*****************************************************************************/
/*                                    Pins                                   */
/*****************************************************************************/

// columns
constexpr int KD1 = 14;
constexpr int KD2 = 15;
constexpr int KD3 = 16;
constexpr int KD4 = 17;
constexpr int KD5 = 18;
constexpr int KD6 = 19;
constexpr int KD7 = 2;
constexpr int KD8 = 3;

// row selectors
constexpr int SA1 = 4;
constexpr int SA2 = 5;
constexpr int SA3 = 6;

constexpr int SEL_A = SA1;
constexpr int SEL_B = SA2;
constexpr int SEL_C = SA3;

// range selector
constexpr int SA4 = 7;
constexpr int SA5 = 8;

constexpr int RS1 = SA4;
constexpr int RS2 = SA5;

/*****************************************************************************/
/*                                Convenience                                */
/*****************************************************************************/
constexpr int col_pins[] = {KD1, KD2, KD3, KD4, KD5, KD6, KD7, KD8};
constexpr int num_col_pins = 8;
constexpr int num_half_pins = 4;

// first hit
constexpr int first_col[] = {KD1, KD2, KD3, KD4};

// second hit
constexpr int second_col[] = { KD5, KD6, KD7, KD8 };

constexpr int selector_pins[] = {SA1, SA2, SA3, SA4, SA5};
constexpr int num_selector_pins = 5;

enum Range { LO = 0, MID, HI};
constexpr Range ranges[] = {Range::LO, Range::MID, Range::HI};

// rows in range
constexpr int rows [] = {8, 8, 6};
constexpr int range_offsets [] = {0,
                                  rows[1] * num_half_pins,
                                  rows[0] * num_half_pins + rows[1] * num_half_pins};

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
  switch(range) {
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
  return (constrain(velocity, min_velocity, max_velocity) - min_velocity) / max_velocity * 127;
}

unsigned long S1[num_keys]; // timer value for first hit or 0
bool S2[num_keys]; // whether key is pressed

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

  digitalWrite(RS1, LOW);
  digitalWrite(RS2, LOW);
  digitalWrite(SEL_A, LOW);
  digitalWrite(SEL_B, LOW);
  digitalWrite(SEL_C, LOW);
}


void loop() {
  int key = 0;

  for (const auto range : ranges) {
    select_range(range);

    for (int row = 0; row < rows[range]; row++) {
      select_row(row);

      // read first row
      for(int i= 0; i < num_half_pins; i++) {
        if (!digitalRead(first_col[i])) {
          if(S1[key] == 0) {
            S1[key] = micros();
          } else {
            if (!S2[key] && !digitalRead(second_col[i])) {
              S2[key] = true;

              auto velocity = 1/static_cast<double>(micros() - S1[key]);
              Serial.print(key);
              Serial.print(" ");
              Serial.println(midi_velocity(velocity), DEC);
            }
          }
        }

        key++;
      }
    }
  }
}
