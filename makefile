.POSIX:

include config.mk

ARDCLI  = arduino-cli

SRCS    = $(wildcard *.c) $(wildcard *.cpp) $(wildcard *.ino)
PRG     = $(shell basename `pwd`)
B_PATH  = $(shell pwd)/build
HEX     = $(PRG).$(subst :,.,$(FQBN)).hex
ELF     = $(PRG).$(subst :,.,$(FQBN)).elf
ARDLIBS = USB-MIDI@1.1.2

$(HEX): $(SRCS)
	$(ARDCLI) compile --fqbn $(FQBN) --build-path $(B_PATH)  $(CURDIR)

upload: $(HEX)
	sudo $(ARDCLI) upload -v --fqbn $(FQBN) --input-dir $(B_PATH) -p $(PORT) $(CURDIR)

deps:
	@$(foreach lib,$(ARDLIBS),$(ARDCLI) lib install $(lib) || true; )

clean:
	$(RM) $(HEX) $(ELF)

.PHONY: all upload deps clean
