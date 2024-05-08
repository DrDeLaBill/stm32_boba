/* Copyright Â© 2024 Georgy E. All rights reserved. */

#include "translate.h"

#include "hal_defs.h"



const char T_TEST_CYRILLIC[] = " !@#\"ABCabc������";


const char T_NO_SENSOR[][TRANSLATE_MAX_LEN] = {
	"NO SENSOR",
	"��� �������"
};
const char T_mode[][TRANSLATE_MAX_LEN] = {
	"mode",
	"�����"
};
const char T_manual[][TRANSLATE_MAX_LEN] = {
	"manual",
	"������"
};
const char T_auto[][TRANSLATE_MAX_LEN] = {
	"auto",
	"����"
};
const char T_surface[][TRANSLATE_MAX_LEN] = {
	"surface",
	"��������"
};
const char T_ground[][TRANSLATE_MAX_LEN] = {
	"ground",
	"�����"
};
const char T_string[][TRANSLATE_MAX_LEN] = {
	"string",
	"������"
};
const char T_error[][TRANSLATE_MAX_LEN] = {
	"error",
	"������"
};
const char T_TARGET[][TRANSLATE_MAX_LEN] = {
	"TARGET",
	"����"
};
const char T_VALUE[][TRANSLATE_MAX_LEN] = {
	"VALUE",
	"��������"
};
const char T_LOADING[][TRANSLATE_MAX_LEN] = {
	"LOADING",
	"��������"
};
const char T_ERROR[][TRANSLATE_MAX_LEN] = {
	"ERROR",
	"������"
};
const char T_service[][TRANSLATE_MAX_LEN] = {
	"service",
	"���������"
};


const char* t(const char phrase[][TRANSLATE_MAX_LEN], uint8_t lang)
{
	assert_param(IS_LANGUAGE(lang));

	return (const char*)phrase[lang];
}
