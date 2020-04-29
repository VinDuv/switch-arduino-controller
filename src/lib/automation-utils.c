/*
 * Automation functions that are not tied to a specific Switch game.
 */

#include "automation-utils.h"


/* Perform controller switching */
void switch_controller(enum switch_mode mode, enum led_state end_led_state)
{
	if (mode == REAL_TO_VIRT) {
		SEND_BUTTON_SEQUENCE(KEEP_LEDS,
			{ BT_L,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Reconnect the controller */
			{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	10 },	/* Wait for reconnection */
		);
	} else {
		go_to_main_menu(KEEP_LEDS);
	}

	/* In both cases, the controller is now connected, the main menu is shown, and the
	   cursor is on the game icon */

	SEND_BUTTON_SEQUENCE((mode == REAL_TO_VIRT) ? KEEP_LEDS : end_led_state,
		{ BT_NONE,		DP_BOTTOM,	SEQ_HOLD,	1  },	/* News button */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	3  },	/* Controllers button */
		{ BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter controllers settings */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	10 },	/* Wait for settings */

		/* Enter change style/order, validating any “interrupt local comm” message */
		{ BT_A,			DP_NEUTRAL,	SEQ_MASH,	16 },
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	20 },	/* Wait for “Press L/R” menu */
	);

	if (mode == REAL_TO_VIRT) {
		SEND_BUTTON_SEQUENCE(KEEP_LEDS,
			{ BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Register as controller 1 */
			{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	15 },	/* Wait for registration */

			{ BT_H,			DP_NEUTRAL,	SEQ_HOLD,	2  },	/* Return to the main menu */
			{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Wait for the main menu */
		);

		go_to_game(end_led_state);
	}
}


/* Go to the main menu, from the currently playing game or menu. */
void go_to_main_menu(enum led_state end_led_state)
{
	SEND_BUTTON_SEQUENCE(KEEP_LEDS,
		{ BT_H,			DP_NEUTRAL,	SEQ_HOLD,	2  },	/* Go to main menu */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Wait for the main menu */
	);
}


/* Go back to the game, from the main menu. */
void go_to_game(enum led_state end_led_state)
{
	SEND_BUTTON_SEQUENCE(end_led_state,
		{ BT_H,			DP_NEUTRAL,	SEQ_HOLD,	2  },	/* Go back to the game */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	40 },	/* Wait for the game */
	);
}


/* Configure the Switch’s clock to manual mode */
void set_clock_to_manual_from_any(bool in_game, enum led_state end_led_state)
{
	if (in_game) {
		go_to_main_menu(KEEP_LEDS);
	}

	SEND_BUTTON_SEQUENCE(KEEP_LEDS,
		{ BT_NONE,		DP_BOTTOM,	SEQ_HOLD,	1  },	/* News button */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	4  },	/* Settings button */
		{ BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter settings */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	20 },	/* Wait for settings */
		{ BT_NONE,		DP_BOTTOM,	SEQ_MASH,	14 },	/* Console settings */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	1  },	/* Update console button */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	5  },	/* Wait for cursor */
		{ BT_NONE,		DP_BOTTOM,	SEQ_MASH,	4  },	/* Date/time */
		{ BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter date/time */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	5  },	/* Wait date/time menu */
		{ BT_NONE,		DP_BOTTOM,	SEQ_MASH,	2  },	/* TZ if auto/time set if man */
		{ BT_NONE,		DP_TOP,		SEQ_MASH,	1  },	/* auto/man if auto, TZ if man */
		{ BT_A,			DP_NEUTRAL,	SEQ_MASH,	1  },	/* Set man if auto, else enter TZ */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	5  },	/* Wait TZ menu, if any */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	2  },	/* Wait for menu */
	);

	go_to_main_menu(in_game ? KEEP_LEDS : end_led_state);

	if (in_game) {
		go_to_game(end_led_state);
	}
}


/* Configure the Switch’s clock to automatic mode */
void set_clock_to_auto_from_manual(bool in_game, enum led_state end_led_state)
{
	if (in_game) {
		go_to_main_menu(KEEP_LEDS);
	}

	SEND_BUTTON_SEQUENCE(NO_LEDS,
		{ BT_NONE,		DP_BOTTOM,	SEQ_HOLD,	1  },	/* News button */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	4  },	/* Settings button */
		{ BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter settings */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	20 },	/* Wait for settings */
		{ BT_NONE,		DP_BOTTOM,	SEQ_MASH,	14 },	/* Console settings */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	1  },	/* Update console button */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	5  },	/* Wait for cursor */
		{ BT_NONE,		DP_BOTTOM,	SEQ_MASH,	4  },	/* Date/time */
		{ BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter date/time */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	5  },	/* Wait date/time menu */
		{ BT_A,			DP_NEUTRAL,	SEQ_MASH,	1  },	/* Set to automatic */
	);

	go_to_main_menu(in_game ? KEEP_LEDS : end_led_state);

	if (in_game) {
		go_to_game(end_led_state);
	}
}


/* Apply an offset to the Switch’s clock’s year. */
void change_clock_year(bool in_game, int8_t offset, enum led_state end_led_state)
{
	uint8_t button;
	uint8_t num;

	if (offset >= 0) {
		/* Increment year */
		button = DP_TOP;
		num = (uint8_t)offset;
	} else {
		/* Decrement year */
		button = DP_BOTTOM;
		num = (uint8_t)(-offset);
	}

	if (in_game) {
		go_to_main_menu(KEEP_LEDS);
	}

	SEND_BUTTON_SEQUENCE(BOTH_LEDS,
		{ BT_NONE,		DP_BOTTOM,	SEQ_HOLD,	1  },	/* News button */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	4  },	/* Settings button */
		{ BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter settings */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	20 },	/* Wait for settings */
		{ BT_NONE,		DP_BOTTOM,	SEQ_MASH,	14 },	/* Console settings */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	1  },	/* Update console button */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	5  },	/* Wait for cursor */
		{ BT_NONE,		DP_BOTTOM,	SEQ_MASH,	4  },	/* Date/time */
		{ BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter date/time */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	5  },	/* Wait date/time menu */
		{ BT_NONE,		DP_BOTTOM,	SEQ_MASH,	2  },	/* Time set */
		{ BT_A,			DP_NEUTRAL,	SEQ_MASH,	1  },	/* Enter time set */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	2  },	/* Wait for menu */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	2  },	/* Go to year */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	2  },	/* Wait for cursor */
		{ BT_NONE,		button,		SEQ_MASH,	num },	/* Change year */
		{ BT_A,			DP_NEUTRAL,	SEQ_MASH,	4  },	/* Go to OK and click it */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	2  },	/* Wait for menu */
	);

	go_to_main_menu(in_game ? KEEP_LEDS : end_led_state);

	if (in_game) {
		go_to_game(end_led_state);
	}
}
