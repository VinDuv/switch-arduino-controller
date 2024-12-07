/*
 * Data persistence. Handles the storage of a uint32 value to EEPROM.
 */

#include "persist.h"

#include <avr/eeprom.h>

/*
 * The code uses the 1024 first bytes of EEPROM to store the value, using a
 * “wear-levelling” mechanism. The EEPROM space is divided into 256 blocks and
 * the value is stored in one of the blocks. When a new value is set, the
 * current block is set to UINT32_MAX and the new value is written to the next
 * block. At startup, the current block to use is found by skipping all blocks
 * that contain UINT32_MAX.
 */
#define BLOCK_COUNT 256

static uint8_t value_pos;

/* Initializes data persistence. */
void init_persist(void) {
	for (uint16_t cur_pos = 0; cur_pos < BLOCK_COUNT ; cur_pos += 1) {
		uintptr_t offset = cur_pos * 4;
		uint32_t value = eeprom_read_dword((const uint32_t*)offset);
		if (value != UINT32_MAX) {
			value_pos = cur_pos;
			return;
		}
	}

	// All blocks are at UINT32_MAX; assume no data was ever written
	value_pos = 0;
}

/* Get the value from EEPROM. */
uint32_t persist_get_value(void)
{
	uintptr_t offset = value_pos * 4;
	return eeprom_read_dword((const uint32_t*)offset);
}


/* Set the value to EEPROM. */
void persist_set_value(uint32_t value)
{
	uintptr_t offset = value_pos * 4;
	eeprom_write_dword((uint32_t*)offset, UINT32_MAX);

	value_pos += 1; // Wraps to 0 when hitting block 256
	offset = value_pos * 4;
	eeprom_write_dword((uint32_t*)offset, value);
}
