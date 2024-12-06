/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _G_SYSTEM_CONFIG_H_
#define _G_SYSTEM_CONFIG_H_


#include "soul.h"


#ifdef __cplusplus
extern "C" {
#endif


#include "soul.h"


#ifdef DEBUG
#   define GSYSTEM_BEDUG (1)
#endif


// #define GSYSTEM_NO_RESTART_W
// #define GSYSTEM_NO_RTC_W
#define GSYSTEM_NO_RTC_CALENDAR_W
// #define GSYSTEM_NO_SYS_TICK_W
// #define GSYSTEM_NO_RAM_W
// #define GSYSTEM_NO_ADC_W
 #define GSYSTEM_NO_I2C_W
// #define GSYSTEM_NO_POWER_W
// #define GSYSTEM_NO_MEMORY_W
// #define GSYSTEM_NO_PLL_CHECK_W

// #define GSYSTEM_NO_I2C

#define GSYSTEM_ADC_VOLTAGE_COUNT (1)

 #define GSYSTEM_FLASH_MODE
//#define GSYSTEM_EEPROM_MODE

// #define GSYSTEM_DS1307_CLOCK

// #define GSYSTEM_TIMER             (TIM1)

#define GSYSTEM_BEDUG_UART        (huart2)

//#define GSYSTEM_I2C               (hi2c2)
//#define GSYSTEM_EEPROM_I2C        GSYSTEM_I2C

#define GSYSTEM_FLASH_SPI         (hspi1)
#define GSYSTEM_FLASH_CS_PORT     (FLASH1_CS_GPIO_Port)
#define GSYSTEM_FLASH_CS_PIN      (FLASH1_CS_Pin)


typedef enum _CUSTOM_SOUL_STATUSES {
	NEED_SERVICE_BACK      = RESERVED_STATUS_01,
	NEED_SERVICE_SAVE      = RESERVED_STATUS_02,
	NEED_SERVICE_UPDATE    = RESERVED_STATUS_03,
	MANUAL_NEED_VALVE_UP   = RESERVED_STATUS_04,
	MANUAL_NEED_VALVE_DOWN = RESERVED_STATUS_05,
	AUTO_NEED_VALVE_UP     = RESERVED_STATUS_06,
	AUTO_NEED_VALVE_DOWN   = RESERVED_STATUS_07
} CUSTOM_SOUL_STATUSES;


#ifdef __cplusplus
}
#endif


#endif
