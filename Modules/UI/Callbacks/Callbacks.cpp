/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Callbacks.h"

#include <cstdio>

#include "main.h"
#include "soul.h"
#include "settings.h"
#include "translate.h"


#define SAMPLING_STEP (50)


void version_callback::click(uint16_t) {}
char* version_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "v%d.%d.%d", DEVICE_MAJOR, DEVICE_MINOR, DEVICE_PATCH);
	return value;
}
char* version_callback::label() { return (char*)t(T_Version, settings.language); }


void language_callback::click(uint16_t button)
{
	if (button == BTN_UP_Pin || button == BTN_DOWN_Pin) {
		settings.language = (settings.language == ENGLISH ? RUSSIAN : ENGLISH);
		set_status(NEED_SERVICE_UPDATE);
	}
}
char* language_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%s", settings.language == ENGLISH ? __STR_DEF2__(ENGLISH) : __STR_DEF2__(RUSSIAN));
	return value;
}
char* language_callback::label()  { return (char*)t(T_Language, settings.language); }


void surface_label_callback::click(uint16_t) {}
char* surface_label_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	return value;
}
char* surface_label_callback::label()  { return (char*)t(T_SURFACE_MODE, settings.language); }

void surface_snstv_callback::click(uint16_t button)
{
	uint8_t tmp = settings.surface_snstv;
	callback_click<uint8_t>(&tmp, 1, button);
	if (tmp >= __arr_len(SENSITIVITY) && button == BTN_UP_Pin) {
		tmp = SENSITIVITY[0];
		return;
	}
	if (tmp >= __arr_len(SENSITIVITY) && button == BTN_DOWN_Pin) {
		tmp = SENSITIVITY[__arr_len(SENSITIVITY)-1];
		return;
	}
	settings.surface_snstv = tmp;
}
char* surface_snstv_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%u", SENSITIVITY[settings.surface_snstv]);
	return value;
}
char* surface_snstv_callback::label()  { return (char*)t(T_Sensitivity, settings.language); }

void surface_delay_callback::click(uint16_t button)
{
	uint8_t tmp = settings.surface_delay;
	if (tmp == 0 && button == BTN_DOWN_Pin) {
		return;
	}
	callback_click<uint8_t>(&tmp, 1, button);
	if (tmp >= SETTINGS_WORK_DELAY_MAX_S && button == BTN_UP_Pin) {
		tmp = SETTINGS_WORK_DELAY_MAX_S;
	}
	settings.surface_delay = tmp;
}
char* surface_delay_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%u %s", settings.surface_delay, t(T_sec, settings.language));
	return value;
}
char* surface_delay_callback::label()  { return (char*)t(T_Delay, settings.language); }


void string_label_callback::click(uint16_t) {}
char* string_label_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	return value;
}
char* string_label_callback::label()  { return (char*)t(T_STRING_MODE, settings.language); }

void string_snstv_callback::click(uint16_t button)
{
	uint8_t tmp = settings.string_snstv;
	callback_click<uint8_t>(&tmp, 1, button);
	if (tmp >= __arr_len(SENSITIVITY) && button == BTN_UP_Pin) {
		tmp = SENSITIVITY[0];
		return;
	}
	if (tmp >= __arr_len(SENSITIVITY) && button == BTN_DOWN_Pin) {
		tmp = SENSITIVITY[__arr_len(SENSITIVITY)-1];
		return;
	}
	settings.string_snstv = tmp;
}
char* string_snstv_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%u", SENSITIVITY[settings.string_snstv]);
	return value;
}
char* string_snstv_callback::label()  { return (char*)t(T_Sensitivity, settings.language); }

void string_delay_callback::click(uint16_t button)
{
	uint8_t tmp = settings.string_delay;
	if (tmp == 0 && button == BTN_DOWN_Pin) {
		return;
	}
	callback_click<uint8_t>(&tmp, 1, button);
	if (tmp >= SETTINGS_WORK_DELAY_MAX_S && button == BTN_UP_Pin) {
		tmp = SETTINGS_WORK_DELAY_MAX_S;
	}
	settings.string_delay = tmp;
}
char* string_delay_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%u %s", settings.string_delay, t(T_sec, settings.language));
	return value;
}
char* string_delay_callback::label()  { return (char*)t(T_Delay, settings.language); }


void bigski_label_callback::click(uint16_t) {}
char* bigski_label_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	return value;
}
char* bigski_label_callback::label()  { return (char*)t(T_BIGSKI_MODE, settings.language); }

void bigski_snstv_callback::click(uint16_t button)
{
	uint8_t tmp = settings.bigski_snstv;
	callback_click<uint8_t>(&tmp, 1, button);
	if (tmp >= __arr_len(SENSITIVITY) && button == BTN_UP_Pin) {
		tmp = SENSITIVITY[0];
		return;
	}
	if (tmp >= __arr_len(SENSITIVITY) && button == BTN_DOWN_Pin) {
		tmp = SENSITIVITY[__arr_len(SENSITIVITY)-1];
		return;
	}
	settings.bigski_snstv = tmp;
}
char* bigski_snstv_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%u", SENSITIVITY[settings.bigski_snstv]);
	return value;
}
char* bigski_snstv_callback::label()  { return (char*)t(T_Sensitivity, settings.language); }

void bigski_delay_callback::click(uint16_t button)
{
	uint8_t tmp = settings.bigski_delay;
	if (tmp == 0 && button == BTN_DOWN_Pin) {
		return;
	}
	callback_click<uint8_t>(&tmp, 1, button);
	if (tmp >= SETTINGS_WORK_DELAY_MAX_S && button == BTN_UP_Pin) {
		tmp = SETTINGS_WORK_DELAY_MAX_S;
	}
	settings.bigski_delay = tmp;
}
char* bigski_delay_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%u %s", settings.bigski_delay, t(T_sec, settings.language));
	return value;
}
char* bigski_delay_callback::label()  { return (char*)t(T_Delay, settings.language); }
