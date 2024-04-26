/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _SENSOR_H_
#define _SENSOR_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>


void sensor_tick();
bool sensor_available();
int16_t get_sensor_value();


#ifdef __cplusplus
}
#endif


#endif
