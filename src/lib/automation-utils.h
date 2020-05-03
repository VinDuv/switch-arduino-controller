/*
 * Automation functions that are not tied to a specific Switch game.
 */

#ifndef AUTOMATION_UTILS_H
#define AUTOMATION_UTILS_H

#include "automation.h"

/* Controller switching mode */
enum switch_mode {
	VIRT_TO_REAL = 0,	/* Virtual to real */
	REAL_TO_VIRT = 1,	/* Real to virtual */
};

/*
 * Switches controller 1 between the user’s “real” controller and the automated “virtual”
 * controller.
 *
 * This is used at startup to connect the virtual controller, and can be used during
 * automation  so the user can temporarily perform manual actions.
 *
 * If mode is VIRT_TO_REAL, the automation will pause the game and activate the
 * controller setup menu. This will allow the user to press A on the real controller
 * to set it up again as controller 1, enabling them to play the game normally.
 *
 * If mode is REAL_TO_VIRT, the expected state is “main menu shown, cursor on the
 * currently playing game”. The virtual controller will (re-)connect (as controller 2),
 * activate the controller setup menu, validate it to become back controller 1, and
 * go back to the game.
 */
void switch_controller(enum switch_mode mode);

/*
 * Go to the main menu, from the currently playing game or menu.
 */
void go_to_main_menu(void);

/*
 * Go back to the game, from the main menu.
 */
void go_to_game(void);

/*
 * Configure the Switch’s clock to manual mode, starting from the game or the
 * the main menu.
 *
 * The clock can be in automatic mode or be already in manual mode.
 *
 * The first parameter indicates that the operations starts in-game (and will end
 * in-game)
 */
void set_clock_to_manual_from_any(bool in_game);

/*
 * Configure the Switch’s clock to automatic mode, starting from the game or the
 * the main menu.
 *
 * The clock needs to be in manual mode.
 *
 * The first parameter indicates that the operations starts in-game (and will end
 * in-game)
 */
void set_clock_to_auto_from_manual(bool in_game);

/*
 * Apply an offset to the Switch’s clock’s year.
 *
 * The clock needs to be in manual mode.
 *
 * The first parameter indicates that the operations starts in-game (and will end
 * in-game)
 * The second parameter is the year offset to apply (-1: substract a year, 1: add a year)
 * The third parameter allows setting the RX/TX LEDs once the operation completes.
 */
void change_clock_year(bool in_game, int8_t offset);

#endif
