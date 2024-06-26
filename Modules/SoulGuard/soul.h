/* Copyright © 2023 Georgy E. All rights reserved. */

#ifndef __SOUL_H
#define __SOUL_H


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#include "gutils.h"
#include "hal_defs.h"


typedef enum _SOUK_STATUS {
	/* Device statuses start */
	STATUSES_START = 0,

	NON_MASKABLE_INTERRUPT,
	HARD_FAULT,
	MEM_MANAGE,
	BUS_FAULT,
	USAGE_FAULT,

	WAIT_LOAD,
	MEMORY_READ_FAULT,
	MEMORY_WRITE_FAULT,
	NEED_MEASURE,
	NEED_STANDBY,
	SETTINGS_INITIALIZED,
	NEED_LOAD_SETTINGS,
	NEED_SAVE_SETTINGS,
	NO_SENSOR,
	MANUAL_NEED_VALVE_UP,
	MANUAL_NEED_VALVE_DOWN,
	AUTO_NEED_VALVE_UP,
	AUTO_NEED_VALVE_DOWN,
	MODBUS_FAULT,
	PUMP_FAULT,
	RTC_FAULT,
	CAN_FAULT,
	NO_BIGSKI,

	NEED_SERVICE_BACK,
	NEED_SERVICE_SAVE,
	NEED_SERVICE_UPDATE,

	/* Device statuses end */
	STATUSES_END,

	/* Device errors start */
	ERRORS_START,

	RCC_ERROR,
	MEMORY_ERROR,
	POWER_ERROR,
	STACK_ERROR,
	LOAD_ERROR,
	RAM_ERROR,
	USB_ERROR,
	SETTINGS_LOAD_ERROR,
	APP_MODE_ERROR,
	VALVE_ERROR,
	ASSERT_ERROR,
	ERROR_HANDLER_CALLED,
	INTERNAL_ERROR,

	/* Device errors end */
	ERRORS_END,

	/* Paste device errors or statuses to the top */
	SOUL_STATUSES_END
} SOUL_STATUS;


typedef struct _soul_t {
	unsigned last_err;
	uint8_t statuses[__div_up(SOUL_STATUSES_END - 1, BITS_IN_BYTE)];
} soul_t;


unsigned get_last_error();
void set_last_error(SOUL_STATUS error);

bool has_errors();

bool is_error(SOUL_STATUS error);
void set_error(SOUL_STATUS error);
void reset_error(SOUL_STATUS error);
unsigned get_first_error();

bool is_status(SOUL_STATUS status);
void set_status(SOUL_STATUS status);
void reset_status(SOUL_STATUS status);


#ifdef __cplusplus
}
#endif


#endif
