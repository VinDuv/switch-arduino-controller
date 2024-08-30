PROGRAMMER=avrispmkii

all: test-usb-repeat-1sec.hex

ifeq ("", "$(wildcard ../../lufa/.git)")
$(error LUFA submodule not downloaded, run 'git submodule update --init' in root)
endif

flash-%: %.hex
	avrdude -p m16u2 -c $(PROGRAMMER) -P usb -U flash:w:$< -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xF4:m -U lock:w:0x0F:m

test-usb-repeat-%.hex: test-usb-repeat-%.c test-usb-repeat.c test-usb-base.c
	$(MAKE) -f Makefile.lufa TARGET=$(subst .hex,,$@) TEST_SRC="$^"
	rm -f *.bin *.elf *.eep *.lss *.map *.sym

clean:
	rm -f *.hex

# Disable automatic removal of intermediary files
.SECONDARY:
