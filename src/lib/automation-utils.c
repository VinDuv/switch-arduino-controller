/*
 * Automation functions that are not tied to a specific Switch game.
 */

#include "automation-utils.h"


/* Perform controller switching */
void switch_controller(enum switch_mode mode)
{
	if (mode == REAL_TO_VIRT) {
		SEND_BUTTON_SEQUENCE(
			{ BT_L,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Reconnect the controller */
			{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	10 },	/* Wait for reconnection */
		);
	} else {
		go_to_main_menu();
	}

	/* In both cases, the controller is now connected, the main menu is shown, and the
	   cursor is on the game icon */

	SEND_BUTTON_SEQUENCE(
		{ BT_NONE,		DP_BOTTOM,	SEQ_HOLD,	1  },	/* Switch Online button or News button (< v11) */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	6  },	/* Sleep button */
		{ BT_NONE,		DP_LEFT,	SEQ_MASH,	2  },	/* Controllers button */
		{ BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter controllers settings */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	10 },	/* Wait for settings */

		/* Enter change style/order, validating any “interrupt local comm” message */
		{ BT_A,			DP_NEUTRAL,	SEQ_MASH,	16 },
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	50 },	/* Wait for “Press L/R” menu */
	);

	if (mode == REAL_TO_VIRT) {
		SEND_BUTTON_SEQUENCE(
			{ BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Register as controller 1 */
			{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	15 },	/* Wait for registration */

			{ BT_H,			DP_NEUTRAL,	SEQ_HOLD,	2  },	/* Return to the main menu */
			{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Wait for the main menu */
		);

		go_to_game();
	}
}


/* Go to the main menu, from the currently playing game or menu. */
void go_to_main_menu(void)
{
	SEND_BUTTON_SEQUENCE(
		{ BT_H,			DP_NEUTRAL,	SEQ_HOLD,	2  },	/* Go to main menu */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Wait for the main menu */
	);
}


/* Go back to the game, from the main menu. */
void go_to_game(void)
{
	SEND_BUTTON_SEQUENCE(
		{ BT_H,			DP_NEUTRAL,	SEQ_HOLD,	2  },	/* Go back to the game */
		{ BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	40 },	/* Wait for the game */
	);
}


/* Configure the Switch’s clock to manual mode */
void set_clock_to_manual_from_any(bool in_game)
{
	if (in_game) {
		go_to_main_menu();
	}

	SEND_BUTTON_SEQUENCE(
		{ BT_NONE,		DP_BOTTOM,	SEQ_HOLD,	1  },	/* Switch Online button or News button (< v11) */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	6  },	/* Sleep button */
		{ BT_NONE,		DP_LEFT,	SEQ_MASH,	1  },	/* Settings button */
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

	go_to_main_menu();

	if (in_game) {
		go_to_game();
	}
}


/* Configure the Switch’s clock to automatic mode */
void set_clock_to_auto_from_manual(bool in_game)
{
	if (in_game) {
		go_to_main_menu();
	}

	SEND_BUTTON_SEQUENCE(
		{ BT_NONE,		DP_BOTTOM,	SEQ_HOLD,	1  },	/* Switch Online button or News button (< v11) */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	6  },	/* Sleep button */
		{ BT_NONE,		DP_LEFT,	SEQ_MASH,	1  },	/* Settings button */
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

	go_to_main_menu();

	if (in_game) {
		go_to_game();
	}
}


/* Apply an offset to the Switch’s clock’s year. */
void change_clock_year(bool in_game, int8_t offset)
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
		go_to_main_menu();
	}

	SEND_BUTTON_SEQUENCE(
		{ BT_NONE,		DP_BOTTOM,	SEQ_HOLD,	1  },	/* Switch Online button or News button (< v11) */
		{ BT_NONE,		DP_RIGHT,	SEQ_MASH,	6  },	/* Sleep button */
		{ BT_NONE,		DP_LEFT,	SEQ_MASH,	1  },	/* Settings button */
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

	go_to_main_menu();

	if (in_game) {
		go_to_game();
	}
}


/* Enter a number of the Switch keyboard. */
void type_number_on_keyboard(uint32_t number)
{
	/* A 32-bit unsigned number can have up to 10 digits (+ terminator) */
	int8_t digits[11];
	int8_t* digits_ptr = digits + 10;

	*digits_ptr = -1;
	if (number == 0) {
		digits_ptr -= 1;
		*digits_ptr = 0;
	} else {
		while (number > 0) {
			digits_ptr -= 1;
			*digits_ptr = number % 10;
			number = number / 10;
		}
	}

	type_digits_on_keyboard(digits_ptr);
}


/* Enter a series of digits on the Switch keyboard. */
void type_digits_on_keyboard(const int8_t* digits)
{
	/* 0 is represented by 10 since it’s after 9 on the keyboard */
	int8_t selected_digit = 1;

	for (;;) {
		int8_t cur_digit = *digits;
		digits += 1;

		if (cur_digit < 0 || cur_digit > 9) {
			break;
		}

		if (cur_digit == 0) {
			cur_digit = 10;
		}

		const int8_t offset = selected_digit - cur_digit;
		if (offset < 0) {
			send_buttons(BT_NONE, DP_RIGHT, -offset);
		} else {
			send_buttons(BT_NONE, DP_LEFT, offset);
		}

		send_buttons(BT_A, DP_NEUTRAL, 1);
		selected_digit = cur_digit;
	}

	// Put the cursor back to the 1 digit
	send_buttons(BT_NONE, DP_LEFT, selected_digit - 1);
}
