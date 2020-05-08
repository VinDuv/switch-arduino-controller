/*
 * Pokémon Sword/Shield automation
 */

#include <util/delay.h>

#include "automation-utils.h"
#include "user-io.h"

/* Static functions */
static void temporary_control(void);
static void repeat_press_a(void);
static void max_raid_menu(void);
static void max_raid_setup(void);
static void light_pillar_setup_with_control(void);
static void repeat_change_raid(void);
static void repeat_change_raid_initial_confirm(void);
static void light_pillar_setup(void);
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
static void release_full_boxes(void);
static void position_box_cursor_topleft(void);
static void for_each_box_pos(bool top_left_start, void (*callback)(void));
static void release_from_box(void);


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
				max_raid_menu();
			break;

			case 4:
				auto_breeding();
			break;

			case 5:
				release_full_boxes();
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
 * Max Raid Battle automation features
 */
void max_raid_menu(void)
{
	/* Set the LEDs so the submenu is identifiable */
	set_leds(TX_LED);
	pause_automation();

	for (;;) {
		uint8_t subfeature = count_button_presses(500, 500);

		for (uint8_t i = 0 ; i < subfeature ; i += 1) {
			beep();
			_delay_ms(200);
		}

		switch (subfeature) {
			case 1: /* Full Max Raid Battle automation */
				max_raid_setup();
				return;
			break;

			case 2: /* Light pillar setup */
				light_pillar_setup_with_control();
				return;
			break;

			case 3: /* Change existing Wishing Piece Raid */
				repeat_change_raid();
				return;
			break;

			default:
				/* Wrong selection */
				delay(100, 200, 1500);
			break;
		}
	}
}


/*
 * Set up a light pillar for the appropriate Raid type (normal/rare) and
 * restart the Raid until the wanted Pokémon is available.
 */
void max_raid_setup(void)
{
	/* First step: getting the appropriate light pillar */
	light_pillar_setup();

	/* While we are out of the game, set the Switch’s clock to manual */
	set_clock_to_manual_from_any(/* in_game */ false);

	/* Get back to the game, wait a bit for it to finish saving and close the
	   text box. */
	set_leds(NO_LEDS);
	go_to_game();

	SEND_BUTTON_SEQUENCE(
		{ BT_B,			DP_NEUTRAL,	SEQ_MASH,	30 },	/* Close the dialogs */
	);

	/* Restore fast text speed */
	set_text_speed(/* fast_speed */ true, /* save */ true);

	/* Open the Raid menu */
	SEND_BUTTON_SEQUENCE(
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter Raid */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Wait */
	);

	repeat_change_raid_initial_confirm();
}


/*
 * From an empty Den, get a light pillar of the appropriate type.
 * Restores the text speed and gives the user control before returning.
 */
void light_pillar_setup_with_control(void)
{
	light_pillar_setup();

	/* Get back to the game, wait a bit for it to finish saving and close
	   the text box. */
	set_leds(NO_LEDS);
	go_to_game();

	SEND_BUTTON_SEQUENCE(
		{ BT_B,			DP_NEUTRAL,	SEQ_MASH,	30 },	/* Close dialogs */
	);

	/* Restore fast text speed */
	set_text_speed(/* fast_speed */ true, /* save */ false);

	/* Give control temporarily before returning */
	temporary_control();
}


/*
 * Repeatedly change the Raid from a Den activated with a Wishing Piece. The
 * first change is done immediately without confirmation, but the next ones ask
 * for user confirmation. Gives back control to the user once they are
 * satisfied with the Raid.
 *
 * Automatically sets the clock manual.
 */
void repeat_change_raid(void)
{
	set_clock_to_manual_from_any(/* in_game */ true);

	/* Open the Raid menu */
	SEND_BUTTON_SEQUENCE(
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Enter Raid */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	25 },	/* Wait */
	);

	/* Perform first change immediately */
	change_raid();

	/* Wait for user confirmation on the next changes, and give them control */
	repeat_change_raid_initial_confirm();
}


/*
 * Repeatedly change the Raid from an open Raid menu created with a Wishing
 * Piece. Waits a bit for the user to confirm each change. Returns once the
 * user is satisfied with the Raid.
 *
 * The clock must be set to manual.
 */
void repeat_change_raid_initial_confirm(void)
{
	for (;;) {
		/* Ask the user to look at the Pokémon in the Max Raid Battle */
		beep();

		/* Do the user wants to do this Raid? */
		if (wait_for_button_timeout(250, 250, 5000)) {
			/* Restore the clock */
			set_leds(NO_LEDS);
			set_clock_to_auto_from_manual(/* in_game */ true);

			/* Give control temporarily */
			temporary_control();

			/* Done */
			break;
		}

		/* Change the Raid */
		change_raid();
	}
}


/*
 * From an empty Den, get a light pillar of the appropriate type.
 * When returning, text speed is slow and the game is paused.
 */
void light_pillar_setup(void)
{
	/* Set text speed to slow so the light pillar appears before the game is
	   saved; also save the game because it’s going to be restarted. */
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
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	120 },	/* Cancelling takes a loong time */
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

	/* Set the LEDs so the submenu is identifiable */
	set_leds(RX_LED);
	pause_automation();

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


/*
 * From the Box menu, releases all Pokémon in the current box, then move
 * to the next Box. User confirmation is asked for each Box. The Boxes must
 * be completely full.
 */
void release_full_boxes(void)
{
	position_box_cursor_topleft();

	bool cursor_topleft = true;

	for (;;) {
		/* Wait for user confirmation */
		beep();
		if (count_button_presses(500, 500) > 1) {
			/* User cancelled, we are done */
			return;
		}

		/* Release the Box content */
		for_each_box_pos(cursor_topleft, &release_from_box);

		/* The cursor position was toggled by the operation */
		cursor_topleft ^= true;

		/* Move to the next Box */
		SEND_BUTTON_SEQUENCE(
			{ BT_R,	DP_NEUTRAL,	SEQ_MASH,	1 },	/* Next Box */
		);
	}
}


/*
 * Position the cursor to the top left Pokémon in the Box menu.
 */
void position_box_cursor_topleft(void)
{
	/* We hold the D-pad to put the cursor in a known position (mashing it does
	   not work since it makes the cursor roll around the edges of the screen).
	   We need to avoir getting the cursor on the Box title since the D-pad
	   will start changing the selected Box. The screen layout also tend to
	   make the cursor stuck in random positions if diagonal directions are
	   used.*/

	SEND_BUTTON_SEQUENCE(
		{ BT_NONE,	DP_BOTTOM,	SEQ_HOLD,	25 },	/* Bottom row */
		{ BT_NONE,	DP_LEFT,	SEQ_HOLD,	25 },	/* Last Pokémon */
		{ BT_NONE,	DP_TOP,		SEQ_MASH,	5  },	/* First team Pokémon */
		{ BT_NONE,	DP_RIGHT,	SEQ_MASH,	1  },	/* Top/Left Box Pokémon */
	);
}


/*
 * Calls a callback after positioning the cursor on each Pokémon in the Box.
 * The starting position can either be the top left Pokémon or the bottom
 * right. The ending cursor position will be the reverse of the starting
 * cursor position.
 */
void for_each_box_pos(bool top_left_start, void (*callback)(void))
{
	/* Do we go left on even rows (row 0, row 2, etc)? */
	uint8_t left_on_even = (top_left_start ? 0 : 1);

	/* Which direction to use to move between rows? */
	enum d_pad_state change_row_dir = (top_left_start ? DP_BOTTOM : DP_TOP);

	for (uint8_t row = 0 ; row < 5 ; row += 1) {
		for (uint8_t col = 0 ; col < 5 ; col += 1) {
			enum d_pad_state move_dir;

			callback();

			if ((row % 2) == left_on_even) {
				move_dir = DP_RIGHT;
			} else {
				move_dir = DP_LEFT;
			}

			SEND_BUTTON_SEQUENCE(
				{ BT_NONE,	move_dir,	SEQ_MASH,	1 },
			);
		}

		callback();
		if (row < 4) {
			SEND_BUTTON_SEQUENCE(
				{ BT_NONE,	change_row_dir,	SEQ_MASH,	1 },
			);
		}
	}
}


/*
 * Release from the Box the Pokémon on which the cursor is on.
 */
void release_from_box(void)
{
	SEND_BUTTON_SEQUENCE(
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	8  },	/* Open menu */
		{ BT_NONE,	DP_TOP,		SEQ_MASH,	2  },	/* Go to option */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	20  },	/* Select option */
		{ BT_NONE,	DP_TOP,		SEQ_MASH,	1  },	/* Go to Yes */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	25  },	/* Validate dialog 1 */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Release 1 */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	10  },	/* Validate dialog 2 */
	);
}
