/*
 * Data persistence. Handles the storage of a uint32 value to EEPROM.
 */

#ifndef PERSIST_H
#define PERSIST_H

#include <stdint.h>

/*
 * Initializes data persistence. Must be called before calling other
 * functions.
 */
void init_persist(void);

/*
 * Get the value from EEPROM.
 */
uint32_t persist_get_value(void);

/*
 * Set the value to EEPROM. The UINT32_MAX value cannot be set.
 */
void persist_set_value(uint32_t value);

#endif
