/*
 * Code for the Arduino’s USB interface. Simulate a Nintendo Switch controller,
 * whose button presses/joystick movements are received on the serial
 * interface.
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <LUFA/Drivers/USB/USB.h>

#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/Peripheral/Serial.h>

#include "usb-descriptors.h"
#include "common.h"


/* Static functions */
static void process_hid_data(void);
static void refresh_and_send_controller_data(void);
static bool refresh_controller_data(void);
static void handle_serial_comm(void);
static void handle_recv_byte(uint8_t recv_byte);
static void panic(uint8_t mode);
static void handle_panic_mode(void);


/*
 * Data sent to the USB host when there is no data available.
 * The data represents no buttons pressed, and control sticks centered.
 */
const uint8_t neutral_controller_data[DATA_SIZE] = {
	0, 0, 0x08, 128, 128, 128, 128, MAGIC_VALUE,
};

/* Output data that will be sent to the host */
static uint8_t out_data[DATA_SIZE];

/* Receive buffer from the main µC */
static uint8_t recv_buffer[DATA_SIZE];

/* Number of bytes in the receive buffer */
static uint8_t recv_buffer_count;

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
	Serial_Init(BAUD, ENABLE_DOUBLESPEED);

	/* Send the initial sync byte to the main µC */
	_delay_ms(11);
	Serial_SendByte(INIT_SYNC_CHAR);

	/* Initialize LUFI */
	USB_Init();

	/* Enable interrupts */
	GlobalInterruptEnable();

	/* Start with a receive buffer full of neutral controller data, so it is
	   taken into account for the first output message to the host, and
	   the ready signal is sent to the main µC */
	memcpy(recv_buffer, neutral_controller_data, sizeof(recv_buffer));
	recv_buffer_count = DATA_SIZE;

	for (;;) {
		/* Handle serial reception */
		handle_serial_comm();

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
void process_hid_data(void)
{
	/* Wait for the device to be configured. */
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	/* Process OUT data (from the host) */
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);

	if (Endpoint_IsOUTReceived()) {
		if (Endpoint_IsReadWriteAllowed()) {
			/* The host data is readable; read it */
			uint8_t recv_data[DATA_SIZE];
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
 * Refreshes the controller data (if needed) and send it to the host.
 * The IN endpoint must be ready before calling this function.
 */
void refresh_and_send_controller_data(void)
{
	uint8_t status;
	static uint8_t send_count = 0;
	bool notify_main_uc = false;

	if (send_count == 0) {
		/* Need to refresh the controller data on this cycle */

		notify_main_uc = refresh_controller_data();
	}

	/* Send the data */
	do {
		status = Endpoint_Write_Stream_LE(out_data, sizeof(out_data), NULL);
	} while (status != ENDPOINT_RWSTREAM_NoError);

	/* Notify the IN data */
	Endpoint_ClearIN();

	if (notify_main_uc) {
		Serial_SendByte(READY_FOR_DATA_CHAR);
	}

	send_count += 1;
	if (send_count == 5) {
		send_count = 0;
	}
}


/*
 * Refresh the controller data to be sent to the host.
 * Returns true if the main µC should be notified that it can send more data.
 */
bool refresh_controller_data(void)
{
	bool notify_main_uc = false;
	static uint8_t prev_recv_count = 0;

	if (panic_mode) {
		return false;
	}

	/* Refresh the controller data */
	if (recv_buffer_count == DATA_SIZE) {
		uint8_t magic_data = recv_buffer[MAGIC_INDEX];

		if ((magic_data & MAGIC_MASK) == MAGIC_VALUE) {
			/* Magic value OK, update LED state and controller data */
			uint8_t new_led_state = 0;

			if (magic_data & MAGIC_TX_STATE) {
				new_led_state |= LEDMASK_TX;
			}

			if (magic_data & MAGIC_RX_STATE) {
				new_led_state |= LEDMASK_RX;
			}

			LEDs_SetAllLEDs(new_led_state);

			/* Don’t copy the magic byte to the controller data, leave it 0 */
			memcpy(out_data, recv_buffer, DATA_SIZE - 1);

			/* Empty the receive buffer and notify the main µC */
			memset(recv_buffer, 0, sizeof(recv_buffer));
			recv_buffer_count = 0;
			notify_main_uc = true;
		} else {
			/* Invalid data received */
			panic(2);
		}

	} else if (memcmp(out_data, neutral_controller_data, DATA_SIZE - 1) != 0) {
		/* The receive buffer was not full, and the output data is not neutral. */
		if (recv_buffer_count == 0) {
			/* The main µC did not send any message on this cycle */
			panic(2);
		} else {
			/* The main µC failed to send a message sufficiently quickly. Note that this
			   is not an error if the current output data is neutral; after sending
			   neutral data, the main µC is allowed to sleep for an arbitrary amount of
			   time. When it starts sending data again, it’s not synchronized with
			   the USB µC, so this function may be called while it’s sending data.

			   When the main µC starts sending non-neutral controller data messages, it’s
			   not supposed to sleep for long periods of time; this means it will stay
			   roughly synchronized with the USB µC’s cycles, and received messages
			   should always be complete when this function is called. */
			panic(3);
		}
	} else if ((recv_buffer_count != 0) && (prev_recv_count == recv_buffer_count)) {
		/* The output data is neutral, and the main µC sent some data, but no data
		   was received during one full cycle. */
		panic(4);
	}

	prev_recv_count = recv_buffer_count;

	return notify_main_uc;
}


/*
 * Receive and process data from the main µC on the serial link.
 */
void handle_serial_comm(void)
{
	int16_t recv_val;

	for (;;) {
		recv_val = Serial_ReceiveByte();
		if (recv_val < 0) {
			return;
		}

		handle_recv_byte((uint8_t)recv_val);
	}
}


/*
 * Handle a received byte from the main µC.
 */
void handle_recv_byte(uint8_t recv_byte)
{
	if ((recv_buffer_count >= (DATA_SIZE) - 1) && (recv_byte == RE_SYNC_QUERY_BYTE)) {
		/* Re-sync query received from the main µC; acknowledge it and
		   reset controller data. The main µC will receive a new data
		   query on the next cycle. */
		Serial_SendByte(RE_SYNC_CHAR);

		panic_mode = 0;

		memcpy(recv_buffer, neutral_controller_data, sizeof(recv_buffer));
		recv_buffer_count = DATA_SIZE;

	} else if (recv_buffer_count < DATA_SIZE) {
		/* Normal data received */
		recv_buffer[recv_buffer_count] = recv_byte;
		recv_buffer_count += 1;

	} else {
		/* Data received while the buffer was full */
		panic(4);
	}
}


/*
 * Enter panic mode. The passed integer determine the number of times
 * the LEDs will blink.
 */
void panic(uint8_t mode)
{
	if (panic_mode) {
		return;
	}

	if (mode == 0) {
		mode = 1;
	}

	panic_mode = mode;

	memcpy(out_data, neutral_controller_data, sizeof(out_data));
}


/*
 * Blink the LEDs in panic mode.
 */
void handle_panic_mode(void)
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
