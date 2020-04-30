/*
 * I/O user interface
 *
 * This file provides functions to provide a basic user interface using the
 * Arduino UNO LED (L) a push button connected beween pins 12 and GND, and
 * an optional buzzer connected between pins 2 and GND.
 *
 * The two other LEDs on the UNO board (RX/TX) are only accessible by the
 * USB interface and can be set using the automation API (see automation.h)
 */

#ifndef USER_IO_H
#define USER_IO_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Initializes the IO interface. Must be called before calling other functions
 * in this file.
 */
void init_led_button(void);

/*
 * Wait the specified amount of time for the button to be pressed. If the
 * button is not pressed, this function returns false. If the button, is
 * pressed, this function waits for the button to be released, and returns
 * true.
 */
bool wait_for_button_timeout(uint16_t led_on_time_ms, uint16_t led_off_time_ms,
	uint16_t timeout_ms);

/*
 * Blink the LED and wait for the user to press the button. Return the number
 * of presses. The user is allowed 500 ms between button presses before this
 * function returns.
 */
uint8_t count_button_presses(uint16_t led_on_time_ms,
	uint16_t led_off_time_ms);

/*
 * Wait a fixed amount of time, blinking the LED (unless led_on_time_ms is 0).
 *
 * Returns the number of times the button was pressed.
 */
uint8_t delay(uint16_t led_on_time_ms, uint16_t led_off_time_ms,
	uint16_t delay_ms);

/*
 * Emit a brief beep the buzzer.
 */
void beep(void);

#endif
