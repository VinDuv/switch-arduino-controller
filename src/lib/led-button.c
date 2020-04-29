#include "led-button.h"

#include <avr/io.h>
#include <util/delay.h>

/* LED (digital pin 13) on port B */
#define PORTB_LED (1 << 5)

/* Button on digital pin 12 port B */
#define PORTB_BUTTON (1 << 4)


/* Initializes the LED/button interface. */
void init_led_button(void)
{
	/* Configure LED as output, button as input */
	DDRB = (DDRB | PORTB_LED) & (~PORTB_BUTTON);

	/* Enable pullup on button */
	PORTB |= PORTB_BUTTON;
}


/* Count the presses on the button during the specified period. */
uint8_t count_button_presses(uint16_t wait_ms)
{
	uint8_t count = 0;
	uint8_t press_time = 0;

	while (wait_ms-- > 0) {
		if (PINB & PORTB_BUTTON) {
			/* Button not pressed; if it was previously held a sufficient time,
			   increment the count */
			if (press_time > 20) {
				count += 1;
			}
			press_time = 0;
		} else {
			/* Button pressed; increment the press time */
			press_time += 1;
		}

		_delay_ms(1);
	}

	if (press_time > 20) {
		count += 1;
	}

	return count;
}


/* Blink the LED with the specified delays and count. */
uint8_t blink_led(uint16_t on_time_ms, uint16_t off_time_ms, uint8_t count,
	bool wait_for_first_press)
{
	uint8_t button_presses = 0;

	while (count > 0) {
		PORTB |= PORTB_LED;
		button_presses += count_button_presses(on_time_ms);

		PORTB &= ~PORTB_LED;
		button_presses += count_button_presses(off_time_ms);

		if (button_presses || !wait_for_first_press) {
			count -= 1;
		}
	}

	return button_presses;
}
