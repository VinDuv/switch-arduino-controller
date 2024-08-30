/*
 * Base for test tools that run on the Arduino’s USB interface. The test tool
 * must provide a refresh_and_send_controller_data function.
 */

#include "test-usb-base.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <LUFA/Drivers/USB/USB.h>

#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/Peripheral/Serial.h>

#include "usb-descriptors.h"

/* Must be defined in another file */
extern void refresh_and_send_controller_data(void);

_Static_assert(sizeof(struct usb_report_data) == 8, "Incorrect sent data size");

/* Static functions */
static void process_hid_data(void);
static void handle_panic_mode(void);

/* Non-zero if in panic mode; indicate the number of LED blinks*/
static uint8_t panic_mode = 0;


/*
 * Entry point
 */
int main(void)
{
	/* Initial setup: Disable watchdog and clock divisor */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Initialize the LEDs and the serial link */
	LEDs_Init();

	/* Initialize LUFI */
	USB_Init();

	/* Enable interrupts */
	GlobalInterruptEnable();

	for (;;) {
		/* Process HID requests/responses */
		process_hid_data();

		/* Run the general USB task */
		USB_USBTask();

		/* Handle LED updating in panic mode */
		handle_panic_mode();
	}
}


/*
 * Process HID data from and to the host.
 */
static void process_hid_data(void)
{
	/* Wait for the device to be configured. */
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	/* Process OUT data (from the host) */
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);

	if (Endpoint_IsOUTReceived()) {
		LEDs_TurnOnLEDs(LEDMASK_RX);
		if (Endpoint_IsReadWriteAllowed()) {
			/* The host data is readable; read it */
			uint8_t recv_data[8];
			uint8_t status;

			do {
				status = Endpoint_Read_Stream_LE(&recv_data, sizeof(recv_data),
					NULL);
			} while (status != ENDPOINT_RWSTREAM_NoError);

			/* The data received is not used. */
		}

		/* Acknowledge the OUT data */
		Endpoint_ClearOUT();
	}

	/* Provide IN data (to the host) */
	Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);

	if (Endpoint_IsINReady()) {
		refresh_and_send_controller_data();
	}
}


/* Enter panic mode. */
void panic(uint8_t mode)
{
	if (panic_mode) {
		return;
	}

	if (mode == 0) {
		mode = 1;
	}

	panic_mode = mode;
}


/*
 * Blink the LEDs in panic mode.
 */
static void handle_panic_mode(void)
{
	static uint32_t count = 0;

	if (!panic_mode) {
		return;
	}

	count += 1;

	uint8_t frame = count >> 16;
	uint8_t blink_range = panic_mode * 2;
	uint8_t wait_range = blink_range + 4;

	if (frame < blink_range) {
		LEDs_SetAllLEDs(((frame % 2) == 0) ? LEDS_ALL_LEDS : LEDS_NO_LEDS);
	} else if (frame < wait_range) {
		LEDs_SetAllLEDs(LEDS_NO_LEDS);
	} else {
		count = 0;
	}
}

/* Called by LUFA to configure the device endpoints */
void EVENT_USB_Device_ConfigurationChanged(void) {
	Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT,
		JOYSTICK_EPSIZE, 1);
	Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT,
		JOYSTICK_EPSIZE, 1);
}
