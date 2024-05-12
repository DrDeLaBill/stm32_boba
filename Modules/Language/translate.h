/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


typedef enum _language_t {
	ENGLISH = 0,
	RUSSIAN
} language_t;


#define TRANSLATE_MAX_LEN (20)
#define IS_LANGUAGE(LANG) ((LANG) == ENGLISH || (LANG) == RUSSIAN)


extern const char T_TEST_CYRILLIC[];


extern const char T_NO_SENSOR[][TRANSLATE_MAX_LEN];
extern const char T_mode[][TRANSLATE_MAX_LEN];
extern const char T_manual[][TRANSLATE_MAX_LEN];
extern const char T_auto[][TRANSLATE_MAX_LEN];
extern const char T_surface[][TRANSLATE_MAX_LEN];
extern const char T_BIGSKY[][TRANSLATE_MAX_LEN];
extern const char T_string[][TRANSLATE_MAX_LEN];
extern const char T_error[][TRANSLATE_MAX_LEN];
extern const char T_TARGET[][TRANSLATE_MAX_LEN];
extern const char T_VALUE[][TRANSLATE_MAX_LEN];
extern const char T_LOADING[][TRANSLATE_MAX_LEN];
extern const char T_ERROR[][TRANSLATE_MAX_LEN];
extern const char T_service[][TRANSLATE_MAX_LEN];


const char* t(const char phrase[][TRANSLATE_MAX_LEN], uint8_t lang);


#ifdef __cplusplus
}
#endif



#endif
