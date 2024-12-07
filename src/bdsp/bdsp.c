/*
 * Pokémon Brilliant Diamond/Shining Pearl automation
 */

#include <util/delay.h>

#include "automation-utils.h"
#include "user-io.h"
#include "persist.h"

/* Static functions */
static void temporary_control(void);
static void display_reset_count(void);
static void zero_reset_count(void);
static void shiny_arceus_hunting(void);
static void reset_game(void);
static uint32_t get_reset_count(void);


int main(void)
{
	init_automation();
	init_led_button();
	init_persist();

	/* Initial beep to confirm that the buzzer works */
	beep();

	/* Wait for the user to press the button (should be on the Switch main menu) */
	count_button_presses(100, 100);

	/* Set the virtual controller as controller 1 */
	switch_controller(REAL_TO_VIRT);

	for (;;) {
		/* Set the LEDs, and make sure automation is paused while in the
		   menu */
		set_leds(NO_LEDS);
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
				display_reset_count();
			break;

			case 3:
				zero_reset_count();
			break;

			case 4:
				shiny_arceus_hunting();
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
 * Shows the number of resets on the screen by using the Mystery Gift Code
 * on-screen keyboard.
 */
void display_reset_count(void)
{
	set_leds(TX_LED);

	SEND_BUTTON_SEQUENCE(
		{ BT_X,		DP_NEUTRAL,	SEQ_HOLD,	5 },	/* Display menu */
		{ BT_NONE,	DP_RIGHT,	SEQ_HOLD,	30 },	/* Go right */
		{ BT_NONE,	DP_BOTTOM, 	SEQ_HOLD,	20 },	/* Go down */
		/* Cursor now on Mystery gift */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Enter Mystery gift */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	50 },	/* Wait for menu */
		{ BT_NONE,	DP_BOTTOM,	SEQ_MASH,	1 },	/* Move to Via Code */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	5 },	/* Confirm */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	20 },	/* Wait for warning */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	5 },	/* Confirm warning */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1 },	/* Release all */
	);

	/* Wait for Internet connection */
	_delay_ms(5000);

	/* If this got the game connected to the internet, there is an additional
	   dialog that can be exited with A or B before the keyboard shows up. If
	   the game was already connected, the keyboard is displayed directly; in
	   that situation, pressing A will type a 1 and pressing B will close the
	   keyboard. */
	SEND_BUTTON_SEQUENCE(
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Confirm “Connected” ? */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	20 },	/* Wait for keyboard */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Type a 1 */
		/* The keyboard is now displayed, with either “1” or “11” depending on
		   if the game was already connected to the Internet or not. */
		{ BT_B,		DP_NEUTRAL,	SEQ_HOLD,	10 },	/* Clear keyboard input */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	1 },	/* Release all */
	);

	/* Write the number */
	type_number_on_keyboard(get_reset_count());

	/* Wait for user to press the button */
	beep();
	count_button_presses(100, 100);

	/* Exit menus */
	SEND_BUTTON_SEQUENCE(
		{ BT_X,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Exit keyboard */
		{ BT_B,		DP_NEUTRAL, SEQ_MASH,	50 },	/* Exit all menus */
	);
}


/*
 * Sets the reset count to zero, after user confirmation
 */
void zero_reset_count(void)
{
	set_leds(RX_LED);
	pause_automation();

	beep();
	beep();
	if (count_button_presses(200, 200) == 1) {
		persist_set_value(0);
	}
}

/*
 * Repeat Arceus encounters with game resets
 */
void shiny_arceus_hunting(void)
{
	set_leds(BOTH_LEDS);

	for (;;) {
		/* Stick up then neutral */
		send_update(BT_NONE, DP_NEUTRAL, S_TOP, S_NEUTRAL);
		send_update(BT_NONE, DP_NEUTRAL, S_TOP, S_NEUTRAL);
		send_update(BT_NONE, DP_NEUTRAL, S_TOP, S_NEUTRAL);
		send_update(BT_NONE, DP_NEUTRAL, S_TOP, S_NEUTRAL);
		send_update(BT_NONE, DP_NEUTRAL, S_NEUTRAL, S_NEUTRAL);

		/* Wait for the animation to finish */
		_delay_ms(12000);

		SEND_BUTTON_SEQUENCE(
			{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	20 },	/* Mash A */
			{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	125 },	/* Wait a bit */
		);

		beep();
		if (count_button_presses(100, 100) > 1) {
			temporary_control();
			return;
		}

		reset_game();
	}
}


/*
 * Restarts the game.
 * The reset counter is incremented.
 * When this returns, the game should be accepting inputs.
 */
void reset_game(void)
{
	persist_set_value(get_reset_count() + 1);

	SEND_BUTTON_SEQUENCE(
		{ BT_H,		DP_NEUTRAL,	SEQ_HOLD,	3 },	/* Home button */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	20 },	/* Wait for home */
		{ BT_X,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Ask to close game */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	10 },	/* Wait for menu */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 },	/* Confirm close */
		{ BT_NONE,	DP_NEUTRAL, SEQ_HOLD,	40 },	/* Wait for close */
		{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	20 },	/* Relaunch game */
	);

	/* Wait for the game to start */
	_delay_ms(20000);

	SEND_BUTTON_SEQUENCE(
		{ BT_A,		DP_NEUTRAL,	SEQ_MASH,	80 }, /* Validate menu */
	);

	/* Wait for the game to load */
	_delay_ms(9500);
}


/*
 * Get the reset count (capped at 100000 to detect uninitialized storage)
 */
uint32_t get_reset_count(void)
{
	uint32_t value = persist_get_value();

	if (value > 100000) {
		return 0;
	}

	return value;
}
