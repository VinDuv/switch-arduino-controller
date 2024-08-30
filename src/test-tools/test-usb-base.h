/*
 * Functions useable by test tools that run on the Arduino’s USB interface.
 */

#ifndef TEST_USB_BASE_H
#define TEST_USB_BASE_H

#include <stdint.h>

/* Definitions */
/* Stick coordinates */
struct stick_coord {
	uint8_t x; /* X position value */
	uint8_t y; /* Y position value */
};

/* stick coordinates */
#define S_NEUTRAL { 128, 128 }
#define S_RIGHT { 255, 128 }

/* D-pad state */
enum d_pad_state {
    DP_TOP = 0,
    DP_TOP_RIGHT = 1,
    DP_RIGHT = 2,
    DP_BOTTOM_RIGHT = 3,
    DP_BOTTOM = 4,
    DP_BOTTOM_LEFT = 5,
    DP_LEFT = 6,
    DP_TOP_LEFT = 7,
    DP_NEUTRAL = 8,
};

/* Buttons state */
enum button_state {
	BT_NONE = 0x0000, /* No buttons are pressed */
	BT_Y =    0x0001, /* The Y button is pressed */
	BT_B =    0x0002, /* The B button is pressed */
	BT_A =    0x0004, /* The A button is pressed */
	BT_X =    0x0008, /* The X button is pressed */
	BT_L =    0x0010, /* The L button is pressed */
	BT_R =    0x0020, /* The R button is pressed */
	BT_ZL =   0x0040, /* The ZL button is pressed */
	BT_ZR =   0x0080, /* The ZR button is pressed */
	BT_M =    0x0100, /* The -/Select button is pressed */
	BT_P =    0x0200, /* The +/Start button is pressed */
	BT_H =    0x1000, /* The Home button is pressed */
	BT_C =    0x2000, /* The Capture button is pressed */
};

/* Data to send to the USB host, with the length */
struct usb_report_data {
	enum button_state buttons : 16; /* Button state */
	enum d_pad_state d_pad : 8; /* D-pad state */
	struct stick_coord l_stick; /* Left stick X/Y coordinate */
	struct stick_coord r_stick; /* Right stick X/Y coordinate */
	/* Should be set to zero when sent to the host; when this structure is used
	   to store static controller data, this may be used to store a repeat
	   count */
	uint8_t repeat;
};

/*
 * Enter panic mode. The passed integer determine the number of times
 * the LEDs will blink.
 */
void panic(uint8_t mode);

#endif
