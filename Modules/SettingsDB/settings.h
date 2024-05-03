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
#define SW_VERSION  ((uint8_t)0x02)
#define FW_VERSION  ((uint8_t)0x01)
#define CF_VERSION  ((uint8_t)0x01)


#define SETTINGS_DEFUALT_SURFACE_KP       (1.0f)
#define SETTINGS_DEFUALT_SURFACE_KI       (0.3f)
#define SETTINGS_DEFUALT_SURFACE_KD       (0.1f)
#define SETTINGS_DEFAULT_SURFACE_SAMPLING (500)

#define SETTINGS_DEFUALT_GROUND_KP        (1.0f)
#define SETTINGS_DEFUALT_GROUND_KI        (0.3f)
#define SETTINGS_DEFUALT_GROUND_KD        (0.1f)
#define SETTINGS_DEFAULT_GROUND_SAMPLING  (500)

#define SETTINGS_DEFUALT_STRING_KP        (1.0f)
#define SETTINGS_DEFUALT_STRING_KI        (0.3f)
#define SETTINGS_DEFUALT_STRING_KD        (0.1f)
#define SETTINGS_DEFAULT_STRING_SAMPLING  (500)


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

    // Last target sensor value
    int16_t   last_target;

    // Surface mode PID
    // PID coefficients
    float     surface_kp;
    float     surface_ki;
    float     surface_kd;
    // PID sampling
    uint32_t  surface_sampling;

    // Surface mode PID
    // PID coefficients
    float     ground_kp;
    float     ground_ki;
    float     ground_kd;
    // PID sampling
    uint32_t  ground_sampling;

    // Surface mode PID
    // PID coefficients
    float     string_kp;
    float     string_ki;
    float     string_kd;
    // PID sampling
    uint32_t  string_sampling;
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
