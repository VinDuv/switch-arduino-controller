#include "automation.h"

#include "common.h" /* Must be included before setbaud.h (defines BAUD) */

#include <avr/io.h>
#include <util/setbaud.h>
#include <util/delay.h>


/* Data to send to the USB µC */
static struct {
	enum button_state buttons : 16; /* Button state */
	enum d_pad_state d_pad : 8; /* D-pad state */
	struct stick_coord l_stick; /* Left stick X/Y coordinate */
	struct stick_coord r_stick; /* Right stick X/Y coordinate */
	uint8_t magic_and_leds; /* Magic number and TX/RX LED state */
} sent_data;
_Static_assert(sizeof(sent_data) == DATA_SIZE, "Incorrect sent data size");

/*
 * Init the automation: sets up the serial link to the USB µC,
 * and initializes the controller state with default data.
 * Returns true if the USB interface was just plugged in, false if the
 * main microcontroller was reset/reprogrammed afterwards.
 */
bool init_automation(void)
{
	/* Set the serial link speed */
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

	/* Check that ENABLE_DOUBLESPEED matches what was determined by setbaud */
	#if USE_2X != ENABLE_DOUBLESPEED
	#error Change ENABLE_DOUBLESPEED and the UCSR0A enabling next line
	#endif

	UCSR0A &= ~_BV(U2X0); /* This bit must be set iff ENABLE_DOUBLESPEED = 1 */

	/* Enable 8-bit mode, RX, and TX */
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);

	/* Initialize the default data */
	sent_data.buttons = BT_NONE;
	sent_data.d_pad = DP_NEUTRAL;
	sent_data.l_stick = S_NEUTRAL;
	sent_data.r_stick = S_NEUTRAL;
	sent_data.magic_and_leds = MAGIC_VALUE;

	/* Wait 12 ms for initial ready signal */
	_delay_ms(12);
	if (bit_is_set(UCSR0A, RXC0)) {
		/* Retrieve ready signal byte */
		uint8_t received = UDR0;
		if (received == INIT_SYNC_CHAR) {
			/* Initial sync done */
			return true;
		}

		if (received != READY_FOR_DATA_CHAR) {
			/* Invalid character received */
			panic(1);
		}

		/* READY_FOR_DATA_CHAR has been received; this may happen depending on
		   the timing. Continue with the resync procedure. */
	}

	for (uint8_t tries = 0 ; tries < DATA_SIZE ; tries += 1) {
		/* Send resync request */
		loop_until_bit_is_set(UCSR0A, UDRE0);
		UDR0 = RE_SYNC_QUERY_BYTE;

		/* Wait for response */
		_delay_ms(5);

		if (bit_is_set(UCSR0A, RXC0)) {
			/* Retrieve resync signal byte */
			uint8_t received = UDR0;
			if (received == RE_SYNC_CHAR) {
				/* Re-sync done */
				return false;
			}

			/* Invalid character received */
			panic(2);
		}

		/* No response yet, the USB µC receive buffer may be not full yet;
		   continue resync */
	}

	/* Failed to resync, the USB µC is probably hung up */
	panic(3);
}


/* Set the LED state to be sent in the next request */
void set_leds(enum led_state leds)
{
	if (leds & TX_LED) {
		sent_data.magic_and_leds |= MAGIC_TX_STATE;
	} else {
		sent_data.magic_and_leds &= ~MAGIC_TX_STATE;
	}

	if (leds & RX_LED) {
		sent_data.magic_and_leds |= MAGIC_RX_STATE;
	} else {
		sent_data.magic_and_leds &= ~MAGIC_RX_STATE;
	}
}


/* Send an update with new button/controller state */
void send_update(enum button_state buttons, enum d_pad_state d_pad,
	struct stick_coord l_stick, struct stick_coord r_stick)
{
	sent_data.buttons = buttons;
	sent_data.d_pad = d_pad;
	sent_data.l_stick = l_stick;
	sent_data.r_stick = r_stick;

	send_current();
}

/* Send a button sequence */
void send_button_sequence(const struct button_d_pad_state sequence[],
	size_t sequence_length)
{
	for (size_t pos = 0 ; pos < sequence_length ; pos += 1) {
		const struct button_d_pad_state* cur = &sequence[pos];

		uint16_t repeat_count = cur->repeat_count;
		enum seq_mode mode = cur->mode;

		while (repeat_count > 0) {
			sent_data.buttons = cur->buttons;
			sent_data.d_pad = cur->d_pad;

			send_current();

			if (mode == SEQ_MASH) {
				sent_data.buttons = BT_NONE;
				sent_data.d_pad = DP_NEUTRAL;
				send_current();
			}

			repeat_count -= 1;
		}
	}
}


/* Send an update with the current state */
void send_current(void)
{
	/* Wait for ready signal for USB µC */
	loop_until_bit_is_set(UCSR0A, RXC0);

	/* Retrieve ready signal byte */
	uint8_t received = UDR0;
	if (received != READY_FOR_DATA_CHAR) {
		panic(2);
	}

	const char* cur_sent_data = (const char*)&sent_data;

	for (uint8_t idx = 0 ; idx < sizeof(sent_data) ; idx += 1) {
		loop_until_bit_is_set(UCSR0A, UDRE0);
		UDR0 = cur_sent_data[idx];
	}
}


/* Enter panic mode */
void panic(uint8_t mode)
{
	const uint8_t portb_led = (1 << 5);

	/* Ensure the LED is powered on */
	DDRB |= portb_led;

	if (mode == 0) {
		mode = 1;
	}

	for (;;) {
		for (uint8_t count = 0 ; count < mode ; count += 1) {
			PORTB |= portb_led;
			_delay_ms(500);
			PORTB &= ~portb_led;
			_delay_ms(500);
		}
		_delay_ms(1000);
	}
}
