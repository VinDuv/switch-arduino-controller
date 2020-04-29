/*
 * Standalone version of the code for the Arduino’s USB interface. Simulate a Nintendo
 * Switch controller, whose button presses/joystick movements are defined statically
 * in this file. Used for testing.
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <LUFA/Drivers/USB/USB.h>

#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/Peripheral/Serial.h>

#include "usb-descriptors.h"


/* Definitions */
/* Stick coordinates */
struct stick_coord {
	uint8_t x; /* X position value */
	uint8_t y; /* Y position value */
};

/* stick coordinates */
#define S_NEUTRAL { 128, 128 }
#define S_RIGHT { 255, 128 }

/* D-pad state */
enum d_pad_state {
    DP_TOP = 0,
    DP_TOP_RIGHT = 1,
    DP_RIGHT = 2,
    DP_BOTTOM_RIGHT = 3,
    DP_BOTTOM = 4,
    DP_BOTTOM_LEFT = 5,
    DP_LEFT = 6,
    DP_TOP_LEFT = 7,
    DP_NEUTRAL = 8,
};

/* Buttons state */
enum button_state {
	BT_NONE = 0x0000, /* No buttons are pressed */
	BT_Y =    0x0001, /* The Y button is pressed */
	BT_B =    0x0002, /* The B button is pressed */
	BT_A =    0x0004, /* The A button is pressed */
	BT_X =    0x0008, /* The X button is pressed */
	BT_L =    0x0010, /* The L button is pressed */
	BT_R =    0x0020, /* The R button is pressed */
	BT_ZL =   0x0040, /* The ZL button is pressed */
	BT_ZR =   0x0080, /* The ZR button is pressed */
	BT_M =    0x0100, /* The -/Select button is pressed */
	BT_P =    0x0200, /* The +/Start button is pressed */
	BT_H =    0x1000, /* The Home button is pressed */
	BT_C =    0x2000, /* The Capture button is pressed */
};

/* Data to send to the USB host, with the length */
struct usb_report_data {
	enum button_state buttons : 16; /* Button state */
	enum d_pad_state d_pad : 8; /* D-pad state */
	struct stick_coord l_stick; /* Left stick X/Y coordinate */
	struct stick_coord r_stick; /* Right stick X/Y coordinate */
	uint8_t repeat; /* Number of times to send this report */
};
_Static_assert(sizeof(struct usb_report_data) == 8, "Incorrect sent data size");

/* The report data sent to the USB host */
const struct usb_report_data report_data[] = {
	/* Go to the All Software page and waiting for it to show up */
	{ BT_NONE,		DP_NEUTRAL, S_NEUTRAL,	S_NEUTRAL, 10, },
	{ BT_L | BT_R,	DP_NEUTRAL, S_NEUTRAL,	S_NEUTRAL, 10, },
	{ BT_NONE,		DP_NEUTRAL, S_NEUTRAL,	S_NEUTRAL, 10, },
	{ BT_A,			DP_NEUTRAL, S_NEUTRAL,	S_NEUTRAL, 10, },
	{ BT_NONE,		DP_NEUTRAL,	S_NEUTRAL,	S_NEUTRAL, 10, },
	{ BT_H,			DP_NEUTRAL,	S_NEUTRAL,	S_NEUTRAL, 10, },
	{ BT_NONE,		DP_NEUTRAL,	S_NEUTRAL,	S_NEUTRAL, 100, },
	{ BT_NONE,		DP_RIGHT,	S_NEUTRAL,	S_NEUTRAL, 200, },
	{ BT_A,			DP_NEUTRAL, S_NEUTRAL,	S_NEUTRAL, 10, },

	{ BT_NONE,		DP_NEUTRAL,	S_NEUTRAL, 	S_NEUTRAL, 200, },

	#if 0
	/* Using continuous press on the right d-pad takes ~68 reports to go to the sixth
	   icon, because autorepeat takes time to start. Same thing with the L-stick. */
	{ BT_NONE,		DP_RIGHT,	S_NEUTRAL,	S_NEUTRAL, 68, },
	#else
	/* Mashing the right d-pad every 5 reports takes 45 reports to go to the sixth
	   icon. */
	{ BT_NONE,		DP_RIGHT,	S_NEUTRAL,	S_NEUTRAL, 5, },
	{ BT_NONE,		DP_NEUTRAL,	S_NEUTRAL,	S_NEUTRAL, 5, },
	{ BT_NONE,		DP_RIGHT,	S_NEUTRAL,	S_NEUTRAL, 5, },
	{ BT_NONE,		DP_NEUTRAL,	S_NEUTRAL,	S_NEUTRAL, 5, },
	{ BT_NONE,		DP_RIGHT,	S_NEUTRAL,	S_NEUTRAL, 5, },
	{ BT_NONE,		DP_NEUTRAL,	S_NEUTRAL,	S_NEUTRAL, 5, },
	{ BT_NONE,		DP_RIGHT,	S_NEUTRAL,	S_NEUTRAL, 5, },
	{ BT_NONE,		DP_NEUTRAL,	S_NEUTRAL,	S_NEUTRAL, 5, },
	{ BT_NONE,		DP_RIGHT,	S_NEUTRAL,	S_NEUTRAL, 5, },
	#endif

	{ BT_NONE,		DP_NEUTRAL,	S_NEUTRAL,	S_NEUTRAL, 1, },
};

/* Current report data to send */
static uint16_t report_idx = 0;

/* Repeat count of the report data */
static uint8_t repeat_count = 0;

/* Static functions */
static void process_hid_data(void);
static void refresh_and_send_controller_data(void);
static void panic(uint8_t mode);
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

	LEDs_TurnOnLEDs(LEDMASK_TX);

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


/*
 * Refreshes the controller data and send it to the host.
 * The IN endpoint must be ready before calling this function.
 */
static void refresh_and_send_controller_data(void)
{
	struct usb_report_data out_data = report_data[report_idx];
	uint8_t status;
	out_data.repeat = 0;

	/* Send the data */
	do {
		status = Endpoint_Write_Stream_LE(&out_data, sizeof(out_data), NULL);
	} while (status != ENDPOINT_RWSTREAM_NoError);

	/* Notify the IN data */
	Endpoint_ClearIN();

	repeat_count += 1;
	if (repeat_count >= report_data[report_idx].repeat) {
		repeat_count = 0;
		if (report_idx < ((sizeof(report_data) / sizeof(*report_data) - 1))) {
			report_idx += 1;
		}
	}
}


/*
 * Enter panic mode. The passed integer determine the number of times
 * the LEDs will blink.
 */
static void panic(uint8_t mode)
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
