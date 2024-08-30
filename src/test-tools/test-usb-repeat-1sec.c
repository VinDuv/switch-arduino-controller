/*
 * USB interface test tool: Sends D-pad left for 5 reports, then nothing for
 * 120 reports, then D-pad right for 5 reports, then nothing for 120 reports,
 * then repeats.
 * The Switch seems to poll the controller at 125 Hz, so this should result in
 * left-right movement every second (full cycle of 2 seconds)
 * (Note that the Switch seems to always eat the first D-pad press, so this
 * moves the cursor to the right at first)
 */

#include "test-usb-repeat.h"

struct usb_report_data test_report_sequence[] = {
	{ BT_NONE,	DP_LEFT, 	S_NEUTRAL,	S_NEUTRAL, 5 },
	{ BT_NONE,	DP_NEUTRAL, S_NEUTRAL,	S_NEUTRAL, 120 },
	{ BT_NONE,	DP_RIGHT,	S_NEUTRAL,	S_NEUTRAL, 5 },
	{ BT_NONE,	DP_NEUTRAL, S_NEUTRAL,	S_NEUTRAL, 120 },
	REPORT_REQ_END
};
