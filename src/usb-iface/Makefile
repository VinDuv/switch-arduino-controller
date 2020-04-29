# Specific Makefile to build the USB interface program using the LUFA library
# and build system.

MCU = atmega16u2
ARCH = AVR8
BOARD = UNO
F_CPU = 16000000
F_USB = $(F_CPU)
OPTIMIZATION = s
TARGET = usb-iface
ifeq ($(strip $(STANDALONE_USB_IFACE)),)
SRC = usb-iface.c usb-descriptors.c $(LUFA_SRC_USB)
else
$(warning Building the standalone USB interface)
SRC = standalone-usb-iface.c usb-descriptors.c $(LUFA_SRC_USB)
endif

LUFA_PATH = ../../lufa/LUFA
CC_FLAGS = -DUSE_LUFA_CONFIG_HEADER

all:

include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
