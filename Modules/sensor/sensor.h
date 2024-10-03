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
    SENSOR_MODE_BIGSKI,
	SENSOR_MODE_ANGLE
} SENSOR_MODE;


typedef enum _STRING_DIRECTION {
	STR_FORCE_LEFT  = (int8_t)-2,
	STR_LEFT        = (int8_t)-1,
	STR_MIDDLE      = (int8_t)0,
	STR_RIGHT       = (int8_t)1,
	STR_FORCE_RIGHT = (int8_t)2
} STRING_DIRECTION;


typedef enum _CAN_STD_ID {
	NO_STD_ID             = 0x0000,
	LINE_CONTROL_VALUE    = 0x0028,
	ANGLE_CONTROL_VALUE   = 0x0030,
	CONTROL_INIT          = 0x0050,
	ANGLE_SENSOR_VALUE    = 0x0096,
	LINE_SENSOR_1_VALUE   = 0x02AB,
	LINE_SENSOR_C_VALUE   = 0x02A7,
	LINE_SENSOR_3_VALUE   = 0x02A8,
	LINE_SENSOR_STATUS    = 0x02A8,
	LINE_CONTROL_STATUS   = 0x03F0,
	ANGLE_CONTROL_STATUS  = 0x03F1,
	LINE_CONTROL_SETTINGS = 0x07EC,
	LINE_SENSOR_SETTINGS  = 0x07ED,
} CAN_STD_ID;


typedef enum _FRAME_DATA_CMD {
	RELATIVE_VALUE       = 0x02,
	LINE_ABSOLUTE1_VALUE = 0x0A,
	LINE_ABSOLUTE2_VALUE = 0x0B,
} FRAME_DATA_CMD;


#define IS_SENSOR_MODE(MODE) ((MODE) == SENSOR_MODE_SURFACE ||  \
                              (MODE) == SENSOR_MODE_STRING || \
                              (MODE) == SENSOR_MODE_BIGSKI || \
							  (MODE) == SENSOR_MODE_ANGLE)


void sensor_tick();
bool sensor2AB_available();
bool sensor2A7_available();
bool sensor2A8_available();
bool sensor_available();
int16_t get_sensor_value();

int16_t get_sensor_mode_target(SENSOR_MODE mode);
void save_sensor_mode_target();
void reset_sensor_mode_target();

void set_sensor_mode(SENSOR_MODE mode);
SENSOR_MODE get_sensor_mode();
SENSOR_MODE get_sensor_target_mode();

STRING_DIRECTION get_sensor_direction();

uint8_t get_sensor_mode_sensitive();


#ifdef __cplusplus
}
#endif


#endif
