/*
 * LED and button interface
 *
 * This file provides functions to provide a basic user interface using the
 * Arduino UNO LED (L) and a push button connected beween pins 12 and GND.
 *
 * The two other LEDs on the UNO board (RX/TX) are only accessible by the
 * USB interface and can be set using the automation API (see automation.h)
 */

#ifndef LED_BUTTON_H
#define LED_BUTTON_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Initializes the LED/button interface. Must be called before calling
 * other functions in this file.
 */
void init_led_button(void);

/*
 * Count the presses on the button during the specified period.
 * Implements basic debouncing.
 */
uint8_t count_button_presses(uint16_t wait_ms);

/*
 * Blink the LED with the specified delays and count. Return the number of
 * times the button was pressed during that time.
 *
 * If wait_for_first_press is true, the count will only start decrementing
 * after the first button press.
 */
uint8_t blink_led(uint16_t on_time_ms, uint16_t off_time_ms, uint8_t count,
	bool wait_for_first_press);

#endif
