.POSIX:

include config.mk

ARDCLI  = arduino-cli

SRCS    = $(wildcard *.c) $(wildcard *.cpp) $(wildcard *.ino)
PRG     = $(shell basename `pwd`)
B_PATH  = $(shell pwd)/build
HEX     = $(B_PATH)/$(PRG).ino.hex
ELF     = $(B_PATH)/$(PRG).ino.elf
ARDLIBS = USB-MIDI@1.1.2

$(HEX): $(SRCS)
	$(ARDCLI) compile --fqbn $(FQBN) --build-path $(B_PATH)  $(CURDIR)

upload: $(HEX)
	sudo $(ARDCLI) upload -v --fqbn $(FQBN) --input-dir $(B_PATH) -p $(PORT) $(CURDIR)

deps:
	@$(foreach lib,$(ARDLIBS),$(ARDCLI) lib install $(lib) || true; )

clean:
	$(RM) $(HEX) $(ELF)
	$(RM) $(B_PATH)

.PHONY: all upload deps clean

.PHONY: serial
serial:
	sudo picocom $(PORT)
