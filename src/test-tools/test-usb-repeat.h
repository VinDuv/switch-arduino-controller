/*
 * Base for test tools that run on the Arduino’s USB interface and repeat a
 * sequence. The sequence must be defined in a test_report_sequence array in
 * another file.
 */

#ifndef TEST_USB_REPEAT_H
#define TEST_USB_REPEAT_H

#include "test-usb-base.h"

/* Sequence sent by test-usb-repeat. The “repeat” field in usb_report_data is
   used to indicates how many times a report will be sent before moving to the
   next report. The end of the array is signaled by an item with repeat equal to
   zero. */
extern struct usb_report_data test_report_sequence[];

#define REPORT_REQ_END { BT_NONE, DP_NEUTRAL, S_NEUTRAL, S_NEUTRAL, 0 }

#endif
