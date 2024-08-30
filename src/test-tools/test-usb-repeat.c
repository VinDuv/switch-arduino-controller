/*
 * Base for test tools that run on the Arduino’s USB interface and repeat a
 * sequence. The sequence must be defined in a test_report_sequence array in
 * another file.
 */

#include "test-usb-repeat.h"

#include <stddef.h>
#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/USB/USB.h>

const struct usb_report_data neutral_data = { BT_NONE, DP_NEUTRAL, S_NEUTRAL,
	S_NEUTRAL, 0};


/*
 * Called by test-usb-base when the host request new controller data.
 * Sends the controller data specified in the test_report_sequence array,
 * in sequence, with the specified repeat count.
 */
void refresh_and_send_controller_data(void)
{
	struct usb_report_data out_data;
	static const struct usb_report_data* seq_ptr = NULL;
	static uint8_t remaining = 0;

	if (seq_ptr == NULL) {
		/* Initialization; wait 10 cycles before starting the sequence */
		out_data = neutral_data;
		remaining += 1;
		if (remaining >= 10) {
			LEDs_TurnOnLEDs(LEDMASK_TX);
			seq_ptr = &test_report_sequence[0];
			remaining = seq_ptr->repeat;
		}
	} else {
		if (remaining == 0) {
			/* End of current item, go to next item */
			seq_ptr += 1;
			if (seq_ptr->repeat == 0) {
				/* End of sequence, go to beginning */
				seq_ptr = &test_report_sequence[0];
			}

			remaining = seq_ptr->repeat;
			LEDs_ToggleLEDs(LEDMASK_TX);
		}

		out_data = *seq_ptr;
		out_data.repeat = 0;

		remaining -= 1;
	}

	if (remaining == 0) {
		/* Empty sequence is not allowed */
		panic(5);
	}

	uint8_t status;

	/* Send the data */
	do {
		status = Endpoint_Write_Stream_LE(&out_data, sizeof(out_data), NULL);
	} while (status != ENDPOINT_RWSTREAM_NoError);

	/* Notify the IN data */
	Endpoint_ClearIN();
}
