/* Copyright Â© 2024 Georgy E. All rights reserved. */

#include "translate.h"

#include <stdio.h>
#include <string.h>

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
const char T_BIGSKY[][TRANSLATE_MAX_LEN] = {
	"BIGSKY",
	"BIGSKY"
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
const char T_LEFT[][TRANSLATE_MAX_LEN] = {
	"LEFT",
	"�����"
};
const char T_RIGHT[][TRANSLATE_MAX_LEN] = {
	"RIGHT",
	"������"
};
const char T_UNKNOWN_ERROR[][TRANSLATE_MAX_LEN] = {
	"UNKNOWN ERROR",
	"����������� ������"
};
const char T_INTERNAL_ERROR[][TRANSLATE_MAX_LEN] = {
	"INTERNAL ERROR",
	"���������� ������"
};
const char T_MEMORY_ERROR[][TRANSLATE_MAX_LEN] = {
	"MEMORY ERROR",
	"������ ������"
};
const char T_POWER_ERROR[][TRANSLATE_MAX_LEN] = {
	"POWER ERROR",
	"������ �������"
};
const char T_STACK_ERROR[][TRANSLATE_MAX_LEN] = {
	"STACK ERROR",
	"������ �����"
};
const char T_LOAD_ERROR[][TRANSLATE_MAX_LEN] = {
	"LOAD ERROR",
	"������ ��������"
};
const char T_RAM_ERROR[][TRANSLATE_MAX_LEN] = {
	"RAM ERROR",
	"������ ���"
};
const char T_SETTINGS_ERROR[][TRANSLATE_MAX_LEN] = {
	"SETTINGS ERROR",
	"������ ��������"
};
const char T_APP_ERROR[][TRANSLATE_MAX_LEN] = {
	"APPLICATION ERROR",
	"������ ����������"
};
const char T_VALVE_ERROR[][TRANSLATE_MAX_LEN] = {
	"VALVE ERROR",
	"������ �������"
};


const char* t(const char phrase[][TRANSLATE_MAX_LEN], uint8_t lang)
{
	assert_param(IS_LANGUAGE(lang));

	return (const char*)phrase[lang];
};

const char* get_string_error(SOUL_STATUS error, uint8_t lang)
{
	static char error_line[2*TRANSLATE_MAX_LEN + 2] = "";
	memset(error_line, 0, sizeof(error_line));
	snprintf(
		error_line,
		sizeof(error_line) - 1,
		"%s",
		t(T_UNKNOWN_ERROR, lang)
	);

	if (error <= ERRORS_START && error >= ERRORS_END) {
		return error_line;
	};

	const char* error_ptr = NULL;
	switch (error)
	{
	case INTERNAL_ERROR:
		error_ptr = T_INTERNAL_ERROR;
		break;
	case MEMORY_ERROR:
		error_ptr = T_MEMORY_ERROR;
		break;
	case POWER_ERROR:
		error_ptr = T_POWER_ERROR;
		break;
	case STACK_ERROR:
		error_ptr = T_STACK_ERROR;
		break;
	case LOAD_ERROR:
		error_ptr = T_LOAD_ERROR;
		break;
	case RAM_ERROR:
		error_ptr = T_RAM_ERROR;
		break;
	case USB_ERROR:
		error_ptr = T_UNKNOWN_ERROR;
		break;
	case SETTINGS_LOAD_ERROR:
		error_ptr = T_LOAD_ERROR;
		break;
	case APP_MODE_ERROR:
		error_ptr = T_APP_ERROR;
		break;
	case VALVE_ERROR:
		error_ptr = T_VALVE_ERROR;
		break;
	default:
		error_ptr = T_UNKNOWN_ERROR;
		break;
	};

	memset(error_line, 0, sizeof(error_line));
	snprintf(
		error_line,
		sizeof(error_line) - 1,
		"%s",
		t(error_ptr, lang)
	);

	return error_line;
}