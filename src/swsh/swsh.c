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
static void auto_breeding(void);
static void reposition_player(bool first_time);
static void go_to_nursery_helper(void);
static void get_egg(void);
static void move_in_circles(uint16_t cycles, bool go_up_first);
static bool hatch_egg(void);

int main(void)
{
	init_automation();
	init_led_button();

	/* Initial beep to confirm that the buzzer works */
	beep();

	/* Wait for the user to press the button (should be on the Switch main menu) */
	count_button_presses(100, 100);

	/* Set the virtual controller as controller 1 */
	switch_controller(REAL_TO_VIRT);

	for (;;) {
		/* Set the LEDs, and make sure automation is paused while in the
		   menu */
		set_leds(BOTH_LEDS);
		pause_automation();

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

			case 4:
				auto_breeding();
			break;

			default:
				/* Wrong selection */
				delay(100, 200, 1500);
			break;
		}
	}
}


/*
 * Temporary gives back control to the user by performing controller switch.
 */
void temporary_control(void)
{
	set_leds(NO_LEDS);

	/* Allow the user to connect their controller back as controller 1 */
	switch_controller(VIRT_TO_REAL);

	/* Wait for the user to press the button (should be on the Switch main menu) */
	count_button_presses(100, 100);

	/* Set the virtual controller as controller 1 */
	switch_controller(REAL_TO_VIRT);
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
	set_clock_to_manual_from_any(/* in_game */ false);

	/* Get back to the game, wait a bit for it to finish saving and close the text box. */
	set_leds(NO_LEDS);
	go_to_game();

	SEND_BUTTON_SEQUENCE(
		{ BT_B,			DP_NEUTRAL,	SEQ_MASH,	30 },	/* Close all “save” dialogs */
	);

	/* Restore fast text speed */
	set_text_speed(/* fast_speed */ true, /* save */ true);

	/* Open the Raid menu */
	SEND_BUTTON_SEQUENCE(
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
			set_clock_to_auto_from_manual(/* in_game */ true);

			/* Give back control */
			switch_controller(VIRT_TO_REAL);

			/* Wait for the user to press the button */
			uint8_t result = count_button_presses(100, 100);

			/* Get back control */
			switch_controller(REAL_TO_VIRT);

			if (result == 1) {
				/* One press: done */
				break;
			}

			/* Pause the game, change the clock to manual again, and restart the game */
			go_to_main_menu();
			set_clock_to_manual_from_any(/* in_game */ false);
			restart_game();

			/* Enter the Raid again; we need to change it because it’s the same as the one
			   that was present just after the Wishing piece was dropped */
			SEND_BUTTON_SEQUENCE(
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
	SEND_BUTTON_SEQUENCE(
		{ BT_X,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Open menu */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	25 },	/* Wait for menu */
		{ BT_NONE,	DP_TOPLEFT, SEQ_HOLD,	25 },	/* Move to top/left position */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1  },	/* Release the buttons */

		{ BT_NONE,	DP_BOTTOM,	SEQ_MASH,	1 },	/* Move to Map position */
		{ BT_NONE,	DP_LEFT,	SEQ_MASH,	1 },	/* Move to Parameters position */

		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Enter Parameters */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	26 },	/* Wait for menu */

		{ BT_NONE,	dir,		SEQ_MASH,	2 },	/* Select speed */

		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	dely },	/* Validate parameters */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1 },	/* Release A to advance */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	dely },	/* Validate parameters */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1 },	/* Release A to advance */
		{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },	/* Validate dialog */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	25 },	/* Wait for menu */
	);

	if (save) {
		SEND_BUTTON_SEQUENCE(
			{ BT_R,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Open Save menu */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	30 },	/* Wait for menu */
			{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },	/* Save the game */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	80 },	/* Wait for save/menu closing */
		);
	} else {
		SEND_BUTTON_SEQUENCE(
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
	SEND_BUTTON_SEQUENCE(
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
	SEND_BUTTON_SEQUENCE(
		{ BT_X,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Ask to close game */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	20 },	/* Wait for menu */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Confirm close */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	80 },	/* Wait for close */
		{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },	/* Relaunch game */
	);

	/* Wait for the game to start */
	_delay_ms(17000);

	SEND_BUTTON_SEQUENCE(
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
	change_clock_year(/* in_game */ true, /* offset */ -1);

	/* Start the raid, but prepare to cancel it */
	SEND_BUTTON_SEQUENCE(
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter “multiple combat” */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	55 },	/* Wait */
		{ BT_B,		DP_NEUTRAL,	SEQ_HOLD,	5  },	/* Open cancel menu (speed up text) */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	10 },	/* Wait a bit */
	);

	/* Set the clock forward */
	change_clock_year(/* in_game */ true, /* offset */ +1);

	/* Cancel the raid (exiting it), then re-enter it */
	SEND_BUTTON_SEQUENCE(
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


/*
 * Automatically get Eggs from the Bridge Field Pokémon Nursery and hatch them.
 */
void auto_breeding(void)
{
	/* Hatching time for each Egg cycles */
	const struct {
		uint16_t hatch_time; /* Time passed spinning for the Egg to hatch */
		uint16_t wait_time; /* Time passed spinning to get another Egg */
	} egg_cycles[] = {
		{ 350, 150 },	/*  5 Egg cycles, approx. 64 Eggs/hour */
		{ 560,  0 },	/* 10 Egg cycles, approx. 60 Eggs/hour */
		{ 1100, 0 },	/* 15 Egg cycles, approx. 50 Eggs/hour */
		{ 1400, 0 },	/* 20 Egg cycles, approx. 40 Eggs/hour */
		{ 1750, 0 },	/* 25 Egg cycles, approx. 33 Eggs/hour */
		{ 2050, 0 },	/* 30 Egg cycles, approx. 30 Eggs/hour */
		{ 2400, 0 },	/* 35 Egg cycles, approx. 24 Eggs/hour */
		{ 2700, 0 },	/* 40 Egg cycles, approx. 22 Eggs/hour */
	};

	/* Note that the automation is mashing B while on the bike, so its speed is irregular.
	   This explains while the spin time is not linear compared to the Egg cycles. */

	/* Select the egg cycle */
	uint16_t hatch_time;
	uint16_t wait_time;

	for (;;) {
		uint8_t cycle_idx = count_button_presses(500, 500) - 1;

		if (cycle_idx < (sizeof(egg_cycles) / sizeof(*egg_cycles))) {
			/* Selection OK, beep once per press */
			for (uint8_t i = 0 ; i <= cycle_idx ; i += 1) {
				beep();
				_delay_ms(200);
			}

			hatch_time = egg_cycles[cycle_idx].hatch_time;
			wait_time = egg_cycles[cycle_idx].wait_time;

			break;
		}

		/* Wrong selection */
		delay(100, 200, 1500);
	}

	/* FIXME: Find a way to ensure the player character is on their bike instead of just
	   toggling the state. For now, just require the player to start on the bike. */
	#ifdef PUT_PLAYER_ON_BIKE
	SEND_BUTTON_SEQUENCE(
		{ BT_P,		DP_NEUTRAL,	SEQ_HOLD,	1  }, /* Get on bike */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	25 }, /* Wait for bike animation to finish */
	);
	#endif

	reposition_player(/* first_time */ true);
	go_to_nursery_helper();

	/* We do not known if an egg is already available, so we just spin the first time */
	move_in_circles(hatch_time + wait_time, /* go_up_first */ true);

	for (;;) {
		reposition_player(/* first_time */ false);
		go_to_nursery_helper();
		get_egg();
		move_in_circles(hatch_time, /* go_up_first */ true);

		if (hatch_egg()) {
			/* Operation stopped by the user */
			break;
		}

		if (wait_time) {
			move_in_circles(wait_time, /* go_up_first */ false);
		}
	}

	/* Give temporary control before returning to the menu */
	temporary_control();
}


/*
 * Use the Flying Taxi to warp to the current position (the nursery) to reposition the
 * player.
 *
 * first_time must be true the first time this function is called; it will then put the
 * X menu’s cursor on the Map icon. It will also set the text speed to Fast.
 */
void reposition_player(bool first_time)
{
	/* Uses held A button to makes the text go faster. */

	set_leds(NO_LEDS);

	if (first_time) {
		SEND_BUTTON_SEQUENCE(
			{ BT_X,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Open menu */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	25 },	/* Wait for menu */
			{ BT_NONE,	DP_TOPLEFT, SEQ_HOLD,	25 },	/* Move to top/left position */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1  },	/* Release the buttons */

			{ BT_NONE,	DP_BOTTOM,	SEQ_MASH,	1 },	/* Move to Map position */
			{ BT_NONE,	DP_LEFT,	SEQ_MASH,	1 },	/* Move to Parameters position */

			{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Enter Parameters */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	26 },	/* Wait for menu */

			{ BT_NONE,	DP_RIGHT,	SEQ_MASH,	2 },	/* Select speed */

			{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	10 },	/* Validate parameters */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1 },	/* Release A to advance */
			{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	10 },	/* Validate parameters */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1 },	/* Release A to advance */
			{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },	/* Validate dialog */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	25 },	/* Wait for menu */

			{ BT_NONE,	DP_RIGHT,	SEQ_MASH,	1 },	/* Move to Map position */
		);
	} else {
		SEND_BUTTON_SEQUENCE(
			{ BT_X,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Open menu */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	25 },	/* Wait for menu */
		);
	}

	SEND_BUTTON_SEQUENCE(
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Open map */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	55 },	/* Wait for map */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	15 },	/* Warp? */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1  },	/* Release A */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Accept */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	60 },	/* Wait for warp to complete */
	);
}


/*
 * Go in front of the nursery helper.
 * reposition_player must be called first.
 */
void go_to_nursery_helper(void)
{
	set_leds(TX_LED);

	send_update(BT_NONE, DP_NEUTRAL, S_SCALED(S_BOTLEFT, 25), S_NEUTRAL);

	for (uint8_t i = 0 ; i < 21 ; i += 1) {
		send_update(BT_NONE, DP_NEUTRAL, S_BOTTOM, S_NEUTRAL);
	}

	send_update(BT_NONE, DP_NEUTRAL, S_RIGHT, S_NEUTRAL);
	send_update(BT_NONE, DP_NEUTRAL, S_RIGHT, S_NEUTRAL);

	/* Reset the sticks and wait for the player to be standing still */
	pause_automation();
	_delay_ms(400);
}


/*
 * Get an Egg from the nursery helper.
 * go_to_nursery_helper must be called first.
 */
void get_egg(void)
{
	SEND_BUTTON_SEQUENCE(
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	10 },	/* Wait after movement */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	15 },	/* Open “accept egg” dialog */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Release A */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Accept egg */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	75 },	/* Wait for dialog  */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Open “what do you want” dialog */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	50 },	/* Wait for dialog */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	20 },	/* Choose “include in team” */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Release A */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Open team dialog */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	45 },	/* Wait for dialog */
		{ BT_NONE,	DP_BOTTOM,	SEQ_MASH,	1  },	/* Go to second Pokémon */
		{ BT_A,		DP_BOTTOM,	SEQ_HOLD,	65 },	/* Select second Pokémon */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Release A */
		{ BT_A,		DP_BOTTOM,	SEQ_HOLD,	35 },	/* Validate “… sent to box” dialog */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Release A */
		{ BT_A,		DP_BOTTOM,	SEQ_MASH,	1  },	/* Validate “Take care” dialog */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Wait for dialog to close */
	);
}

/*
 * Move in circle for the specified number of cycles.
 * go_to_nursery_helper must be called first for correct positioning.
 */
void move_in_circles(uint16_t cycles, bool go_up_first)
{
	set_leds(RX_LED);

	if (go_up_first) {
		send_update(BT_NONE, DP_NEUTRAL, S_SCALED(S_TOP, 25), S_NEUTRAL);

		for (uint8_t i = 0 ; i < 10 ; i += 1) {
			send_update(BT_NONE,	DP_NEUTRAL, S_TOP, S_NEUTRAL);
			send_update(BT_B, 		DP_NEUTRAL, S_TOP, S_NEUTRAL);
		}

		for (uint8_t i = 0 ; i < 50 ; i += 1) {
			send_update(BT_NONE, DP_NEUTRAL, S_TOPRIGHT, S_NEUTRAL);
		}
	}

	for (uint16_t i = 0 ; i < (cycles / 2) ; i += 1) {
		send_update(BT_NONE,	DP_NEUTRAL, S_RIGHT, S_LEFT);
		send_update(BT_B,		DP_NEUTRAL, S_RIGHT, S_LEFT);
	}

	/* Reset sticks position */
	pause_automation();
}


/*
 * Hatch an egg. The “What?” dialog must be shown on screen.
 * Returns true if the process was interrupted by the user.
 */
bool hatch_egg(void)
{
	set_leds(BOTH_LEDS);

	SEND_BUTTON_SEQUENCE(
		{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },	/* Validate “What?” dialog */
	)

	/* Egg hatching animation */
	if (delay(250, 250, 12500)) {
		return true;
	}

	SEND_BUTTON_SEQUENCE(
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Speed up egg dialog text */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Release A */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Validate egg dialog */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	80 },	/* Wait for fadeout */
	)

	return false;
}
