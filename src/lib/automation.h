/*
 * Switch controller automation features
 *
 * The USB interface emulating the Switch controller expects new data to be
 * sent to it on each “cycle” (5 USB reports, ~40 ms when plugged to a Switch).
 * The functions defined in this state allows sending new controller data at
 * the correct rate. The sent controller data also includes the state of the
 * TX and RX LEDs on the Arduino board, which are controlled by the USB
 * interface.
 */

#ifndef AUTOMATION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * Init the automation; must be called early at program start.
 * Returns true if the USB interface was just plugged in, false if the
 * main microcontroller was reset/reprogrammed afterwards.
 */
bool init_automation(void);

/* LED state */
enum led_state {
	NO_LEDS = 0x00, /* Turn off RX and TX LEDs */
	TX_LED = 0x01, /* Turn on TX LED */
	RX_LED = 0x02, /* Turn on RX LED */
	BOTH_LEDS = 0x03, /* Turn on RX and TX LEDs */
};

/* Stick coordinates */
struct stick_coord {
	uint8_t x; /* X position value */
	uint8_t y; /* Y position value */
};

/* Return stick coordinates from X/Y positions. Usable in
   initializer lists or function calls. */
#define S_COORD(X, Y) ((struct stick_coord){(X), (Y)})

/* Predefined stick coordinates */
#define S_TOP S_COORD(128, 0)
#define S_TOPRIGHT S_COORD(219, 37)
#define S_RIGHT S_COORD(255, 128)
#define S_BOTRIGHT S_COORD(219, 219)
#define S_BOTTOM S_COORD(128, 255)
#define S_BOTLEFT S_COORD(37, 219)
#define S_LEFT S_COORD(0, 128)
#define S_TOPLEFT S_COORD(37, 37)
#define S_NEUTRAL S_COORD(128, 128)

/* Internal macro for coordinate scaling */
#define S_SCALE_XY(COORD, M, VAL) \
	(uint8_t)((((int16_t)COORD.M - 128) * VAL) / 255 + 128)

/* Scale down the inclination of the stick, on a scale from 0 (not inclined,
   neutral position) to 255 (original value). For instance, S_TOP is the
   stick inclined 100% to the up direction, S_SCALED(S_TOP, 128) is the stick
   inclined 50% to the up direction, S_SCALED(S_TOP, 25) is the stick inclined
   10% to the up direction, etc */
#define S_SCALED(COORD, VAL) \
	S_COORD(S_SCALE_XY(COORD, x, VAL), S_SCALE_XY(COORD, y, VAL))

/* D-pad state */
enum d_pad_state {
    DP_TOP = 0,
    DP_TOPRIGHT = 1,
    DP_RIGHT = 2,
    DP_BOTRIGHT = 3,
    DP_BOTTOM = 4,
    DP_BOTLEFT = 5,
    DP_LEFT = 6,
    DP_TOPLEFT = 7,
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

/* Sequence run mode */
enum seq_mode {
	SEQ_HOLD = 0, /* Keep the buttons held during the specified number of cycles */
	SEQ_MASH = 1, /* Insert a “release all buttons” cycle after each cycle */
};

/* Button and D-pad state, for sequence runs */
struct button_d_pad_state {
	enum button_state buttons : 16; /* Buttons */
	enum d_pad_state d_pad : 4; /* D-pad */
	enum seq_mode mode : 1;
	uint16_t repeat_count : 11; /* Number of cycles (max 2047) */
};

/* Set the LED state to be sent during the next update. */
void set_leds(enum led_state leds);

/* Send an update with new button/controller state */
void send_update(enum button_state buttons, enum d_pad_state d_pad,
	struct stick_coord l_stick, struct stick_coord r_stick);

/*
 * Send a button sequence.
 * The first parameter is a pointer to an array of states to run in sequence.
 * The second parameter is the number of entries in the array.
 */
void send_button_sequence(const struct button_d_pad_state sequence[],
	size_t sequence_length);

/*
 * Macro to simplify the use of send_button_sequence.
 *
 * Example usage: SEND_BUTTON_SEQUENCE({ BUTTON_A, DP_NEUTRAL, SEQ_HOLD, 5},
 * { NO_BUTTONS, DP_TOP, SEQ_HOLD, 1 });
 */
#define SEND_BUTTON_SEQUENCE(FIRST_STATE, ...) \
	send_button_sequence((struct button_d_pad_state[]){ \
		FIRST_STATE, __VA_ARGS__ }, sizeof((struct button_d_pad_state[]){ \
		FIRST_STATE, __VA_ARGS__ }) / \
		sizeof(struct button_d_pad_state));

/*
 * Send an update that reset the button/controller state to a neutral state
 * (no buttons pressed, sticks centered). This needs to be called if no updates
 * are going to be sent for a long period (more than a cycle length).
 */
inline void pause_automation(void) {
	send_update(BT_NONE, DP_NEUTRAL, S_NEUTRAL, S_NEUTRAL);
}

/*
 * Send an update with the current state. This be used after a call to set_leds
 * to send the new LED state immediately.
 */
void send_current(void);

/* Enter panic mode; the L LED will repetitively blink the number of times
   specified in the parameters. Values 0 to 3 are used internally by the
   automation functions and should not be specified. Never returns. */
void panic(uint8_t mode) __attribute__((noreturn));

#endif
