/* Copyright © 2023 Georgy E. All rights reserved. */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "gutils.h"


#define DEVICE_MAJOR (0)
#define DEVICE_MINOR (1)
#define DEVICE_PATCH (0)
/*
 * Device types:
 * 0x0001 - Dispenser
 * 0x0002 - Gas station
 * 0x0003 - Logger
 * 0x0004 - B.O.B.A.
 */
#define DEVICE_TYPE ((uint16_t)0x0004)
#define SW_VERSION  ((uint8_t)0x01)
#define FW_VERSION  ((uint8_t)0x01)
#define CF_VERSION  ((uint8_t)0x01)


#define SETTINGS_BIGSKI_COUNT          (3)
#define SETTINGS_BANDS_COUNT           (10)
#define SETTNNGS_WORK_DELAY_DEFAULT_S  (0)
#define SETTINGS_WORK_DELAY_MAX_S      (40)


extern const uint8_t  SENSITIVITY[SETTINGS_BANDS_COUNT];
extern const uint32_t SENSITIVITY_DELAY_MS[__arr_len(SENSITIVITY)];
extern const uint16_t DEAD_BANDS_MMx10[__arr_len(SENSITIVITY)];
extern const uint16_t PROP_BANDS_MMx10[__arr_len(SENSITIVITY)];


typedef enum _SettingsStatus {
    SETTINGS_OK = 0,
    SETTINGS_ERROR
} SettingsStatus;


typedef struct __attribute__((packed)) _settings_t  {
    // Device type
	uint16_t  dv_type;
	// Software version
    uint8_t   sw_id;
    // Firmware version
    uint8_t   fw_id;
    // Configuration version
    uint32_t  cf_id;

    // Language
    uint8_t   language;

    // Surface mode sensitivity
    uint8_t   surface_snstv;
    // Surface mode work delay in ms
    uint8_t   surface_delay;
    // Last surface target sensor value
    int16_t   surface_target;

    // String mode sensitivity
    uint8_t   string_snstv;
    // String mode work delay in ms
    uint8_t   string_delay;
    // Last string target sensor value
    int16_t   string_target;

    // BIGSKI mode sensitivity
    uint8_t   bigski_snstv;
    // BIGSKI mode work delay in ms
    uint8_t   bigski_delay;
    // Last BIGSKI target sensor value
    int16_t   bigski_target[SETTINGS_BIGSKI_COUNT];
} settings_t;


extern settings_t settings;


/* copy settings to the target */
settings_t* settings_get();
/* copy settings from the target */
void settings_set(settings_t* other);
/* reset current settings to default */
void settings_reset(settings_t* other);

uint32_t settings_size();

bool settings_check(settings_t* other);
void settings_repair(settings_t* other);

void settings_show();


#ifdef __cplusplus
}
#endif


#endif
