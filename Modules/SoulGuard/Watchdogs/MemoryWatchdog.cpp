/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Watchdogs.h"

#include <random>

#include "soul.h"
#include "hal_defs.h"
#include "at24cm01.h"


#define ERRORS_MAX (5)


utl::Timer MemoryWatchdog::timer(SECOND_MS);
uint8_t MemoryWatchdog::errors = 0;


void MemoryWatchdog::check()
{
	if (timer.wait()) {
		return;
	}
	timer.start();

	uint8_t data = 0;
	eeprom_status_t status = EEPROM_OK;
	if (is_status(MEMORY_READ_FAULT) || is_status(MEMORY_WRITE_FAULT)) {
		uint32_t address = static_cast<uint32_t>(rand()) % eeprom_get_size();

		status = eeprom_read(address, &data, sizeof(data));
		if (status == EEPROM_OK) {
			reset_status(MEMORY_READ_FAULT);
			status = eeprom_write(address, &data, sizeof(data));
		} else {
			errors++;
		}
		if (status == EEPROM_OK) {
			reset_status(MEMORY_WRITE_FAULT);
			errors = 0;
		} else {
			errors++;
		}
	}

	(errors > ERRORS_MAX) ? set_error(MEMORY_ERROR) : reset_error(MEMORY_ERROR);
}
