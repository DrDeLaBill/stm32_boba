/* Copyright © 2023 Georgy E. All rights reserved. */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#include "main.h"


#define DEVICE_VER1 (0)
#define DEVICE_VER2 (1)
#define DEVICE_VER3 (0)
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


#define SETTINGS_BIGSKI_COUNT             (3)

#define SETTINGS_DEFAULT_PID_MAX          ((uint32_t)1000)

#define SETTINGS_DEFUALT_SURFACE_KP       (0.55f)
#define SETTINGS_DEFUALT_SURFACE_KI       (0.0f)
#define SETTINGS_DEFUALT_SURFACE_KD       (0.0f)
#define SETTINGS_DEFAULT_SURFACE_SAMPLING (50)

#define SETTINGS_DEFUALT_GROUND_KP        (0.55f)
#define SETTINGS_DEFUALT_GROUND_KI        (0.0f)
#define SETTINGS_DEFUALT_GROUND_KD        (0.0f)
#define SETTINGS_DEFAULT_GROUND_SAMPLING  (50)

#define SETTINGS_DEFUALT_STRING_KP        (0.55f)
#define SETTINGS_DEFUALT_STRING_KI        (0.0f)
#define SETTINGS_DEFUALT_STRING_KD        (0.0f)
#define SETTINGS_DEFAULT_STRING_SAMPLING  (50)

#define SETTINGS_PID_MULTIPLIER           ((int)100)


typedef enum _SettingsStatus {
    SETTINGS_OK = 0,
    SETTINGS_ERROR
} SettingsStatus;


typedef struct __attribute__((packed)) _pid_t {
    // PID coefficients
    float    kp;
    float    ki;
    float    kd;
    // PID sampling
    uint32_t sampling;
} mode_t;


typedef struct __attribute__((packed)) _settings_t  {
    // Device type
	uint16_t  dv_type;
	// Software version
    uint8_t   sw_id;
    // Firmware version
    uint8_t   fw_id;
    // Configuration version
    uint32_t  cf_id;

    // Max PID output time (ms)
    int16_t   max_pid_time;

    // Language
    uint8_t   language;

    // Surface mode PID
    mode_t     surface;
    // Last surface target sensor value
    int16_t    surface_target;

    // String mode PID
    mode_t     string;
    // Last string target sensor value
    int16_t    string_target;

    // Ground mode PID
    mode_t     bigski;
    // Last string target sensor value
    int16_t    bigski_target[SETTINGS_BIGSKI_COUNT];
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

void settings_show();


#ifdef __cplusplus
}
#endif


#endif
