CFLAGS=-Wall -Wextra -Werror=overflow -Werror=type-limits -std=c11 -Os -I src/usb-iface -I src/lib

# Optionally add <prog>.hex here so it is built when make is invoked
# without arguments.
all: swsh.hex usb-iface.hex
	@echo "Build done. Use flash-<program name> to flash a file."

# Put program definitions (.o => src/<prog>.elf) here
# make <prog>.hex will generate the final program and make flash-<prog> will
# flash it.
src/swsh.elf: src/swsh/swsh.o src/lib/automation.o src/lib/automation-utils.o src/lib/user-io.o

flash-%: %.hex
	avrdude -p atmega328p -c avrispmkii -P usb -U flash:w:$<:i

flash-usb-iface: usb-iface.hex
	avrdude -p m16u2 -c avrispmkii -P usb -U flash:w:$< -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xF4:m -U lock:w:0x0F:m

restore-usb-iface: UNO-dfu_and_usbserial_combined.hex
	avrdude -p m16u2 -c avrispmkii -P usb -U flash:w:$< -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xF4:m -U lock:w:0x0F:m

usb-iface.hex: lufa/.git src/usb-iface/usb-iface.c src/usb-iface/standalone-usb-iface.c src/usb-iface/usb-descriptors.c
	$(MAKE) -C src/usb-iface usb-iface.hex
	cp src/usb-iface/usb-iface.hex usb-iface.hex

UNO-dfu_and_usbserial_combined.hex:
	curl -O https://raw.githubusercontent.com/arduino/ArduinoCore-avr/master/firmwares/atmegaxxu2/UNO-dfu_and_usbserial_combined.hex

%.hex: src/%.elf
	avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $< $<.eep
	avr-objcopy -O ihex -R .eeprom $< $@

src/%.elf:
	@[ "-n" "$^" ] || (echo "No source files specified to build $@; add a program definition at the top of the Makefile." >&2; exit 1)
	avr-gcc -mmcu=atmega328p -flto -fuse-linker-plugin -Wl,--gc-sections -o $@ $^
	
%.o: %.c
	avr-gcc $(CFLAGS) -mmcu=atmega328p -DF_CPU=16000000 -ffunction-sections -fdata-sections -flto -fuse-linker-plugin -o $@ -c $<

clean:
	rm -f src/{*/*,*}.{o,elf,eep} *.hex
	make -C src/usb-iface clean

lufa/.git: .gitmodules
	@echo "- Initializing/Updating LUFA submodule"
	git submodule update --init

# Disable automatic removal of intermediary files
.SECONDARY:
