CC=avr-gcc
CFLAGS=-Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU)
MCU=atmega48
F_CPU=8000000UL

OBJCOPY=avr-objcopy
BIN_FORMAT=ihex

PORT=usb:
BAUD=10
PROTOCOL=avrispmkII
PART=$(MCU)
AVRDUDE=avrdude -F -V -v -v -v

RM=rm -f

.PHONY: all
all: blink.hex

blink.hex: blink.elf

blink.elf: blink.s

blink.s: blink.c

.PHONY: clean
clean:
	$(RM) blink.elf blink.hex blink.s

.PHONY: upload
upload: blink.hex
	sudo $(AVRDUDE) -c $(PROTOCOL) -p $(PART) -P $(PORT) -b $(BAUD) -U flash:w:$<

verify: blink.hex
	sudo $(AVRDUDE) -c $(PROTOCOL) -p $(PART) -P $(PORT) -b $(BAUD) -U flash:v:$<

fuses:
	sudo $(AVRDUDE) -c $(PROTOCOL) -p $(PART) -P $(PORT) -b $(BAUD) -B 100 -U hfuse:w:0xdf:m
	sudo $(AVRDUDE) -c $(PROTOCOL) -p $(PART) -P $(PORT) -b $(BAUD) -B 100 -U lfuse:w:0xe2:m
	sudo $(AVRDUDE) -c $(PROTOCOL) -p $(PART) -P $(PORT) -b $(BAUD) -B 100 -U efuse:w:0x1:m

%.elf: %.s ; $(CC) $(CFLAGS) -s -o $@ $<

%.s: %.c ; $(CC) $(CFLAGS) -S -o $@ $<

%.hex: %.elf ; $(OBJCOPY) -O $(BIN_FORMAT) -R .eeprom $< $@
