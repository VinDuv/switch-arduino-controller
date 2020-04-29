/*
 * LUFA configuration file.
 */

#ifndef LUFA_CONFIG_H
#define LUFA_CONFIG_H

/* Set USB options statically */
#define USE_STATIC_OPTIONS (USB_DEVICE_OPT_FULLSPEED | USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL)

/* Device mode only */
#define USB_DEVICE_ONLY

/* Store the USB descriptors in Flash memory */
#define USE_FLASH_DESCRIPTORS

/* Static endpoint control size */
#define FIXED_CONTROL_ENDPOINT_SIZE 64

/* Static configurations number */
#define FIXED_NUM_CONFIGURATIONS 1

#endif
