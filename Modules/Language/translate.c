/* Copyright Г‚В© 2024 Georgy E. All rights reserved. */

#include "translate.h"

#include "hal_defs.h"



const char T_TEST_CYRILLIC[] = " !@#\"ABCabcАБВабв";


const char T_NO_SENSOR[][TRANSLATE_MAX_LEN] = {
	"NO SENSOR",
	"НЕТ ДАТЧИКА"
};
const char T_mode[][TRANSLATE_MAX_LEN] = {
	"mode",
	"режим"
};
const char T_manual[][TRANSLATE_MAX_LEN] = {
	"manual",
	"ручной"
};
const char T_auto[][TRANSLATE_MAX_LEN] = {
	"auto",
	"авто"
};
const char T_surface[][TRANSLATE_MAX_LEN] = {
	"surface",
	"покрытие"
};
const char T_ground[][TRANSLATE_MAX_LEN] = {
	"ground",
	"грунт"
};
const char T_string[][TRANSLATE_MAX_LEN] = {
	"string",
	"струна"
};
const char T_error[][TRANSLATE_MAX_LEN] = {
	"error",
	"ошибка"
};
const char T_TARGET[][TRANSLATE_MAX_LEN] = {
	"TARGET",
	"ЦЕЛЬ"
};
const char T_VALUE[][TRANSLATE_MAX_LEN] = {
	"VALUE",
	"ЗНАЧЕНИЕ"
};
const char T_LOADING[][TRANSLATE_MAX_LEN] = {
	"LOADING",
	"ЗАГРУЗКА"
};
const char T_ERROR[][TRANSLATE_MAX_LEN] = {
	"ERROR",
	"ОШИБКА"
};
const char T_service[][TRANSLATE_MAX_LEN] = {
	"service",
	"сервисный"
};


const char* t(const char phrase[][TRANSLATE_MAX_LEN], uint8_t lang)
{
	assert_param(IS_LANGUAGE(lang));

	return (const char*)phrase[lang];
}
