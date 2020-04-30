/*
 * Pokémon Sword/Shield automation
 */

#include <util/delay.h>

#include "automation-utils.h"
#include "user-io.h"

/* Static functions */
static void temporary_control(void);
static void repeat_press_a(void);
static void max_raid(void);
static void set_text_speed(bool fast_speed, bool save);
static void use_wishing_piece_and_pause(void);
static void restart_game(void);
static void change_raid(void);

int main(void)
{
	init_automation();
	init_led_button();

	/* Initial beep to confirm that the buzzer works */
	beep();


	/* Wait for the user to press the button (should be on the Switch main menu) */
	count_button_presses(100, 100);

	/* Set the virtual controller as controller 1 */
	switch_controller(REAL_TO_VIRT, BOTH_LEDS);

	for (;;) {
		/* Feature selection menu */
		uint8_t count = count_button_presses(100, 900);

		for (uint8_t i = 0 ; i < count ; i += 1) {
			beep();
			_delay_ms(100);
		}


		switch (count) {
			case 1:
				temporary_control();
			break;

			case 2:
				repeat_press_a();
			break;

			case 3:
				max_raid();
			break;

			default:
				/* Wrong selection */
				delay(100, 200, 1500);
			break;
		}

		/* Make sure the automation is paused before going back to the menu */
		set_leds(BOTH_LEDS);
		pause_automation();
	}
}


/*
 * Temporary gives back control to the user by performing controller switch.
 */
void temporary_control(void)
{
	set_leds(NO_LEDS);

	/* Allow the user to connect their controller back as controller 1 */
	switch_controller(VIRT_TO_REAL, KEEP_LEDS);

	/* Wait for the user to press the button (should be on the Switch main menu) */
	count_button_presses(100, 100);

	/* Set the virtual controller as controller 1 */
	switch_controller(REAL_TO_VIRT, BOTH_LEDS);
}


/*
 * Press A repetitively until the button is pressed.
 */
static void repeat_press_a(void)
{
	uint8_t count = 0;
	while (delay(0, 0, 50) == 0) {
		switch (count % 4) {
			case 0:
				set_leds(NO_LEDS);
			break;
			case 1:
				set_leds(TX_LED);
			break;
			case 2:
				set_leds(BOTH_LEDS);
			break;
			case 3:
				set_leds(RX_LED);
			break;
		}

		send_update(BT_A, DP_NEUTRAL, S_NEUTRAL, S_NEUTRAL);
		send_update(BT_NONE, DP_NEUTRAL, S_NEUTRAL, S_NEUTRAL);

		count += 1;
	}

	set_leds(NO_LEDS);
	send_update(BT_NONE, DP_NEUTRAL, S_NEUTRAL, S_NEUTRAL);

	_delay_ms(200);
}


/*
 * Allow choosing the Pokémon in a Max Raid Battle with one Wishing Piece.
 */
void max_raid(void)
{
	/* First step: getting the appropriate light pillar */

	/* Set text speed to slow so the light pillar appears before the game is saved; also
	   save the game because it’s going to be restarted. */
	set_text_speed(/* fast_speed */ false, /* save */ true);

	for (;;) {
		/* Ask the user to look at the light pillar color */
		beep();

		/* Drop the Wishing Piece in the den */
		use_wishing_piece_and_pause();

		/* Let the user choose what to do */
		if (wait_for_button_timeout(250, 250, 5000)) {
			/* User confirmed, continue */
			break;
		}

		/* Restart the game to re-try dropping the Wishing Piece */
		restart_game();
	}

	/* While we are out of the game, set the Switch’s clock to manual */
	set_clock_to_manual_from_any(/* in_game */ false, NO_LEDS);

	/* Get back to the game, wait a bit for it to finish saving and close the text box. */
	go_to_game(KEEP_LEDS);

	SEND_BUTTON_SEQUENCE(KEEP_LEDS,
		{ BT_B,			DP_NEUTRAL,	SEQ_MASH,	30 },	/* Close all “save” dialogs */
	);

	/* Restore fast text speed */
	set_text_speed(/* fast_speed */ true, /* save */ true);

	/* Open the Raid menu */
	SEND_BUTTON_SEQUENCE(BOTH_LEDS,
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter Raid */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Wait */
	);

	for (;;) {
		/* Ask the user to look at the Pokémon in the Max Raid Battle */
		beep();

		/* Do the user wants to do this Raid? */
		if (wait_for_button_timeout(250, 250, 5000)) {
			/* Restore the clock */
			set_leds(NO_LEDS);
			set_clock_to_auto_from_manual(/* in_game */ true, KEEP_LEDS);

			/* Give back control */
			switch_controller(VIRT_TO_REAL, KEEP_LEDS);

			/* Wait for the user to press the button */
			uint8_t result = count_button_presses(100, 100);

			/* Get back control */
			switch_controller(REAL_TO_VIRT, KEEP_LEDS);

			if (result == 1) {
				/* One press: done */
				break;
			}

			/* Pause the game, change the clock to manual again, and restart the game */
			go_to_main_menu(KEEP_LEDS);
			set_clock_to_manual_from_any(/* in_game */ false, KEEP_LEDS);
			restart_game();

			/* Enter the Raid again; we need to change it because it’s the same as the one
			   that was present just after the Wishing piece was dropped */
			SEND_BUTTON_SEQUENCE(BOTH_LEDS,
				{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter Raid */
				{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Wait */
			);
		}

		/* Change the Raid */
		change_raid();
	}

}


/*
 * Open the parameters menu, set the text speed, and optionally save the game.
 * This requires the Parameters button on the X menu to be in the lower right corner.
 */
void set_text_speed(bool fast_speed, bool save)
{
	uint8_t dir; /* Direction for speed selection */
	uint8_t dely; /* Menu delay */

	if (fast_speed) {
		dir = DP_RIGHT;
		dely = 10;
	} else {
		dir = DP_LEFT;
		dely = 25;
	}

	/* Uses held A button to makes the text go faster. */
	SEND_BUTTON_SEQUENCE(KEEP_LEDS,
		{ BT_X,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Open menu */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	25 },	/* Wait for menu */
		{ BT_NONE,	DP_TOPLEFT, SEQ_HOLD,	25 },	/* Move to top/left position */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1  },	/* Release the buttons */

		{ BT_NONE,	DP_BOTTOM,	SEQ_MASH,	1 },	/* Move to Map position */
		{ BT_NONE,	DP_LEFT,	SEQ_MASH,	1 },	/* Move to Parameters position */

		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Enter Parameters */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	25 },	/* Wait for menu */

		{ BT_NONE,	dir,		SEQ_MASH,	2 },	/* Select speed */

		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	dely },	/* Validate parameters */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1 },	/* Release A to advance */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	dely },	/* Validate parameters */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1 },	/* Release A to advance */
		{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },	/* Validate dialog */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	25 },	/* Wait for menu */
	);

	if (save) {
		SEND_BUTTON_SEQUENCE(KEEP_LEDS,
			{ BT_R,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Open Save menu */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	30 },	/* Wait for menu */
			{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },	/* Save the game */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	80 },	/* Wait for save/menu closing */
		);
	} else {
		SEND_BUTTON_SEQUENCE(KEEP_LEDS,
			{ BT_B,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Close the menu */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	30 },	/* Wait for menu closing */
		);
	}
}


/*
 * Drop a Wishing Piece in a den, and pause the game before it saves. This allows seeing
 * the light ray before allowing to game to save.
 *
 * When this function returns, the game can be either exited (which saves the Wishing
 * Piece) or resumed (which consumes it as the game is saved).
 */
void use_wishing_piece_and_pause(void)
{
	/* A is held to speed up the text */
	SEND_BUTTON_SEQUENCE(TX_LED,
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	35 },	/* First confirmation dialog */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1  },	/* Release A */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	35 },	/* Validate dialog 1, open 2nd */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1 }, 	/* Release A */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Validate second dialog */

		/* It takes approximatively 2.6 seconds (80 video frames @ 30 FPS) between
		   the A press and the “Save completed” dialog to show up (at slow text
		   speed). The Home button needs to be pressed in the interval, but the timing
		   of the events seems to vary depending on the runs. If Home is pressed too
		   quickly, it has no effect, or the light pillar will not be visible; if it
		   is pressed too late, the save has already been completed and the Raid will
		   not be able to be restarted.

		   An wait delay of 1.6 seconds (40 cycles) is used here, which hopefully
		   works every time. Note that if the the player in not directly in front of
		   the Raid den, it will turn to face it, which will add an additional delay.
		   Pressing Home should still work in this case, but the light pillar may not
		   be visible.

		   Home is held for a little while; this prevents the game from occasionally
		   skipping it.
		 */

		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	40 },	/* Let the light pillar appear */
		{ BT_H,		DP_NEUTRAL,	SEQ_HOLD,	3  },	/* Interrupt save */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1  },	/* Release buttons */
	);
}


/*
 * Restart the game (from the Switch main menu)
 */
void restart_game(void)
{
	SEND_BUTTON_SEQUENCE(RX_LED,
		{ BT_X,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Ask to close game */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	20 },	/* Wait for menu */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Confirm close */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	80 },	/* Wait for close */
		{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },	/* Relaunch game */
	);

	/* Wait for the game to start */
	_delay_ms(17000);

	SEND_BUTTON_SEQUENCE(KEEP_LEDS,
		{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },	/* Validate start screen */
	);

	/* Wait a bit more than necessary for the game to load, so background loading will
	   hopefully not interfere with the automation. */
	_delay_ms(9000);
}


/*
 * From an open raid menu created with a Wishing Piece, start an stop the raid while
 * changing the system clock. This will cause the Pokémon in the raid to change.
 *
 * The clock must be set to manual.
 */
void change_raid(void)
{
	/* Set the clock backwards */
	set_leds(TX_LED);
	change_clock_year(/* in_game */ true, /* offset */ -1, KEEP_LEDS);

	/* Start the raid, but prepare to cancel it */
	SEND_BUTTON_SEQUENCE(RX_LED,
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter “multiple combat” */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	55 },	/* Wait */
		{ BT_B,		DP_NEUTRAL,	SEQ_HOLD,	5  },	/* Open cancel menu (speed up text) */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	10 },	/* Wait a bit */
	);

	/* Set the clock forward */
	change_clock_year(/* in_game */ true, /* offset */ +1, KEEP_LEDS);

	/* Cancel the raid (exiting it), then re-enter it */
	SEND_BUTTON_SEQUENCE(BOTH_LEDS,
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Cancel raid */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	100 },	/* Cancelling takes a loong time */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	15 },	/* Absorb the watts (speed up text) */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Release the A button */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	15 },	/* Validate second message */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Release the A button */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter raid */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Wait */
	);
}
