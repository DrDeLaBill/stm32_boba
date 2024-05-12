/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _SENSOR_H_
#define _SENSOR_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>


#define SENSOR_BEDUG (false)


typedef enum _SENSOR_MODE {
    SENSOR_MODE_SURFACE = 0x01,
    SENSOR_MODE_STRING,
    SENSOR_MODE_BIGSKY
} SENSOR_MODE;


#define IS_SENSOR_MODE(MODE) ((MODE) == SENSOR_MODE_SURFACE ||  \
                              (MODE) == SENSOR_MODE_STRING || \
                              (MODE) == SENSOR_MODE_BIGSKY)


void sensor_tick();
bool sensor_available();
int16_t get_sensor2A7_value();
int16_t get_sensor2A8_value();
int16_t get_sensor2AB_value();
int16_t get_sensor_average_value();

void set_sensor_mode(SENSOR_MODE mode);
SENSOR_MODE get_sensor_mode();


#ifdef __cplusplus
}
#endif


#endif
