/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

#include "soul.h"


typedef enum _language_t {
	ENGLISH = 0,
	RUSSIAN
} language_t;


#define TRANSLATE_MAX_LEN (25)
#define IS_LANGUAGE(LANG) ((LANG) == ENGLISH || (LANG) == RUSSIAN)


extern const char T_TEST_CYRILLIC[];


extern const char T_NO_SENSOR[][TRANSLATE_MAX_LEN];
extern const char T_MODE[][TRANSLATE_MAX_LEN];
extern const char T_MANUAL[][TRANSLATE_MAX_LEN];
extern const char T_AUTO[][TRANSLATE_MAX_LEN];
extern const char T_surface[][TRANSLATE_MAX_LEN];
extern const char T_string[][TRANSLATE_MAX_LEN];
extern const char T_BIGSKI[][TRANSLATE_MAX_LEN];
extern const char T_SURFACE_MODE[][TRANSLATE_MAX_LEN];
extern const char T_STRING_MODE[][TRANSLATE_MAX_LEN];
extern const char T_BIGSKI_MODE[][TRANSLATE_MAX_LEN];
extern const char T_error[][TRANSLATE_MAX_LEN];
extern const char T_TARGET[][TRANSLATE_MAX_LEN];
extern const char T_VALUE[][TRANSLATE_MAX_LEN];
extern const char T_LOADING[][TRANSLATE_MAX_LEN];
extern const char T_ERROR[][TRANSLATE_MAX_LEN];
extern const char T_SERVICE[][TRANSLATE_MAX_LEN];
extern const char T_LEFT[][TRANSLATE_MAX_LEN];
extern const char T_RIGHT[][TRANSLATE_MAX_LEN];
extern const char T_sec[][TRANSLATE_MAX_LEN];
extern const char T_Version[][TRANSLATE_MAX_LEN];
extern const char T_Sensitivity[][TRANSLATE_MAX_LEN];
extern const char T_Delay[][TRANSLATE_MAX_LEN];
extern const char T_Language[][TRANSLATE_MAX_LEN];
extern const char T_UPDATING_SETTINGS[][TRANSLATE_MAX_LEN];
extern const char T_RESETING_CHANGES[][TRANSLATE_MAX_LEN];

extern const char T_MEMORY_ERROR[][TRANSLATE_MAX_LEN];
extern const char T_POWER_ERROR[][TRANSLATE_MAX_LEN];
extern const char T_STACK_ERROR[][TRANSLATE_MAX_LEN];
extern const char T_LOAD_ERROR[][TRANSLATE_MAX_LEN];
extern const char T_RAM_ERROR[][TRANSLATE_MAX_LEN];
extern const char T_SETTINGS_LOAD_ERROR[][TRANSLATE_MAX_LEN];
extern const char T_APP_ERROR[][TRANSLATE_MAX_LEN];
extern const char T_VALVE_ERROR[][TRANSLATE_MAX_LEN];


const char* t(const char phrase[][TRANSLATE_MAX_LEN], uint8_t lang);

const char* get_string_error(SOUL_STATUS error, uint8_t lang);


#ifdef __cplusplus
}
#endif



#endif
