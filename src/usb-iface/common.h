/*
 * Shared definition between the main µC and the USB µC code
 */

#ifndef COMMON_H
#define COMMON_H

/* Baud rate of the serial link */
#define BAUD 9600

/* Double speed mode (must be 0 or 1) */
#define ENABLE_DOUBLESPEED 0

/* Size of the messages transferred between the µC (which is also the size
   of a message sent to the USB host) */
#define DATA_SIZE 8

/* Byte index in the message with the magic value */
#define MAGIC_INDEX (DATA_SIZE - 1)

/* Mask of the bytes containing the magic value */
#define MAGIC_MASK 0xFC

/* Magic value */
#define MAGIC_VALUE 0xAC

/* TX LED state in the magic value byte */
#define MAGIC_TX_STATE 0x01

/* RX LED state in the magic value byte */
#define MAGIC_RX_STATE 0x02

/* Character sent by the USB µC for initial sync */
#define INIT_SYNC_CHAR 'I'

/* Character sent by the USB µC for re-sync */
#define RE_SYNC_CHAR 'S'

/* Character sent by the USB µC when data can be sent by the main µC */
#define READY_FOR_DATA_CHAR 'R'

/* Byte repetitively sent by the main µC to request re-sync */
#define RE_SYNC_QUERY_BYTE 0x00

#endif
