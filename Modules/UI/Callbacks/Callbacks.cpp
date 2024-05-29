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


void label_callback::click(uint16_t) {}
char* label_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	return value;
}


void language_callback::click(uint16_t button)
{
	if (button == BTN_UP_Pin || button == BTN_DOWN_Pin) {
		settings.language = (settings.language == ENGLISH ? RUSSIAN : ENGLISH);
	}
}
char* language_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%s", settings.language == ENGLISH ? __STR_DEF2__(ENGLISH) : __STR_DEF2__(RUSSIAN));
	return value;
}


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
