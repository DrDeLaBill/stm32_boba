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
const char T_MODE[][TRANSLATE_MAX_LEN] = {
	"MODE",
	"�����"
};
const char T_MANUAL[][TRANSLATE_MAX_LEN] = {
	"MANUAL",
	"������"
};
const char T_AUTO[][TRANSLATE_MAX_LEN] = {
	"AUTO",
	"����"
};
const char T_surface[][TRANSLATE_MAX_LEN] = {
	"surface",
	"��������"
};
const char T_string[][TRANSLATE_MAX_LEN] = {
	"string",
	"������"
};
const char T_BIGSKI[][TRANSLATE_MAX_LEN] = {
	"BIGSKI",
	"BIGSKI"
};
const char T_SURFACE_MODE[][TRANSLATE_MAX_LEN] = {
	"SURFACE MODE",
	"����� ��������"
};
const char T_STRING_MODE[][TRANSLATE_MAX_LEN] = {
	"STRING MODE",
	"����� ������"
};
const char T_BIGSKI_MODE[][TRANSLATE_MAX_LEN] = {
	"BIGSKI MODE",
	"BIGSKI �����"
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
const char T_SERVICE[][TRANSLATE_MAX_LEN] = {
	"SERVICE",
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
const char T_sec[][TRANSLATE_MAX_LEN] = {
	"sec",
	"���"
};
const char T_Version[][TRANSLATE_MAX_LEN] = {
	"Version",
	"������"
};
const char T_Sensitivity[][TRANSLATE_MAX_LEN] = {
	"Sensitivity",
	"����������������"
};
const char T_Delay[][TRANSLATE_MAX_LEN] = {
	"Delay",
	"��������"
};
const char T_Language[][TRANSLATE_MAX_LEN] = {
	"Language",
	"����"
};
const char T_UPDATING_SETTINGS[][TRANSLATE_MAX_LEN] = {
	"UPDATING SETTINGS",
	"���������� ��������"
};
const char T_RESETING_CHANGES[][TRANSLATE_MAX_LEN] = {
	"RESETING CHANGES",
	"����� ���������"
};
const char T_RESET_ERROR[][TRANSLATE_MAX_LEN] = {
	"RESET ERROR",
	"������ ������"
};


const char T_INTERRUPT_ERROR[][TRANSLATE_MAX_LEN] = {
	"INTERRUPT ERROR",
	"������ ����������"
};
const char T_HARD_FAULT[][TRANSLATE_MAX_LEN] = {
	"HARD FAULT",
	"����������� ������"
};
const char T_MEMORY_MANAGE[][TRANSLATE_MAX_LEN] = {
	"MEMORY MANAGE",
	"���������� �������"
};
const char T_MEMORY_ACCESS_ERROR[][TRANSLATE_MAX_LEN] = {
	"MEMORY ACCESS ERROR",
	"������ ������� � ������"
};
const char T_ILLEGAL_STATE[][TRANSLATE_MAX_LEN] = {
	"ILLEGAL STATE",
	"������������ ���������"
};


const char T_UNKNOWN_ERROR[][TRANSLATE_MAX_LEN] = {
	"UNKNOWN ERROR",
	"����������� ������"
};
const char T_RCC_ERROR[][TRANSLATE_MAX_LEN] = {
	"RCC ERROR",
	"������ RCC"
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
const char T_SETTINGS_LOAD_ERROR[][TRANSLATE_MAX_LEN] = {
	"SETTINGS LOAD ERROR",
	"������ �������� ��������"
};
const char T_APP_ERROR[][TRANSLATE_MAX_LEN] = {
	"APPLICATION ERROR",
	"������ ����������"
};
const char T_VALVE_ERROR[][TRANSLATE_MAX_LEN] = {
	"VALVE ERROR",
	"������ �������"
};
const char T_ASSERT_ERROR[][TRANSLATE_MAX_LEN] = {
	"VALIDATION ERROR",
	"������ ���������"
};
const char T_ERROR_HANDLER_CALLED[][TRANSLATE_MAX_LEN] = {
	"PERIPHERY ERROR",
	"������ ���������"
};
const char T_INTERNAL_ERROR[][TRANSLATE_MAX_LEN] = {
	"INTERNAL ERROR",
	"���������� ������"
};


const char* t(const char phrase[][TRANSLATE_MAX_LEN], uint8_t lang)
{
	assert_param(IS_LANGUAGE(lang));

	return (const char*)phrase[lang];
}

const char* get_string_error(SOUL_STATUS error, uint8_t lang)
{
	if (error <= ERRORS_START && error >= ERRORS_END && error != RCC_FAULT) {
		return t(T_UNKNOWN_ERROR, lang);
	};

	switch (error) {
#ifdef DEBUG
	case NON_MASKABLE_INTERRUPT:
		return t(T_INTERNAL_ERROR, lang);
	case HARD_FAULT:
		return t(T_HARD_FAULT, lang);
	case MEM_MANAGE:
		return t(T_MEMORY_MANAGE, lang);
	case BUS_FAULT:
		return t(T_MEMORY_ACCESS_ERROR, lang);
	case USAGE_FAULT:
		return t(T_ILLEGAL_STATE, lang);
	case RCC_FAULT:
	case RCC_ERROR:
		return t(T_RCC_ERROR, lang);
	case STACK_ERROR:
		return t(T_STACK_ERROR, lang);
	case RAM_ERROR:
		return t(T_RAM_ERROR, lang);
	case USB_ERROR:
		return t(T_UNKNOWN_ERROR, lang);
	case APP_MODE_ERROR:
		return t(T_APP_ERROR, lang);
	case ASSERT_ERROR:
		return t(T_ASSERT_ERROR, lang);
	case ERROR_HANDLER_CALLED:
		return t(T_ERROR_HANDLER_CALLED, lang);
#endif
	case POWER_ERROR:
		return t(T_POWER_ERROR, lang);
	case MEMORY_ERROR:
		return t(T_MEMORY_ERROR, lang);
	case VALVE_ERROR:
		return t(T_VALVE_ERROR, lang);
	case LOAD_ERROR:
		return t(T_LOAD_ERROR, lang);
	case SETTINGS_LOAD_ERROR:
		return t(T_SETTINGS_LOAD_ERROR, lang);
	case INTERNAL_ERROR:
		return t(T_INTERNAL_ERROR, lang);
	default:
		return t(T_UNKNOWN_ERROR, lang);
	};
}