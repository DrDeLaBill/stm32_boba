/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _SENSOR_H_
#define _SENSOR_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>


#define SENSOR_BEDUG (0)


typedef enum _SENSOR_MODE {
    SENSOR_MODE_SURFACE = 0x01,
    SENSOR_MODE_STRING,
    SENSOR_MODE_BIGSKI
} SENSOR_MODE;


#define IS_SENSOR_MODE(MODE) ((MODE) == SENSOR_MODE_SURFACE ||  \
                              (MODE) == SENSOR_MODE_STRING || \
                              (MODE) == SENSOR_MODE_BIGSKI)


void sensor_tick();
bool sensor_available();
int16_t get_sensor2A7_value();
int16_t get_sensor2A8_value();
int16_t get_sensor2AB_value();
int16_t get_sensor_average();

int16_t get_sensor_mode_target();
void save_sensor_mode_target();
void reset_sensor_mode_target();

bool sensor2AB_available();
bool sensor2A7_available();
bool sensor2A8_available();

void set_sensor_mode(SENSOR_MODE mode);
SENSOR_MODE get_sensor_mode();


#ifdef __cplusplus
}
#endif


#endif
