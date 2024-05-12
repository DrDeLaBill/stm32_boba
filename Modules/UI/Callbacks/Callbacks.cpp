/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Callbacks.h"

#include <cstdio>

#include "main.h"
#include "soul.h"
#include "settings.h"
#include "translate.h"


void version_callback::click(uint16_t button)
{
	switch (button) {
	case BTN_ENTER_Pin:
	case BTN_UP_Pin:
	case BTN_DOWN_Pin:
	case BTN_MODE_Pin:
	case BTN_F1_Pin:
	case BTN_F2_Pin:
	case BTN_F3_Pin:
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown callback handler");
#endif
		set_error(INTERNAL_ERROR);
		Error_Handler();
		break;
	};
}

char* version_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "v%ld.%ld.%ld", DEVICE_VER1, DEVICE_VER2, DEVICE_VER3);
	return value;
}


void label_callback::click(uint16_t button)
{
	switch (button) {
	case BTN_ENTER_Pin:
	case BTN_UP_Pin:
	case BTN_DOWN_Pin:
	case BTN_MODE_Pin:
	case BTN_F1_Pin:
	case BTN_F2_Pin:
	case BTN_F3_Pin:
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown callback handler");
#endif
		set_error(INTERNAL_ERROR);
		Error_Handler();
		break;
	};
}

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


void surface_kp_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.surface_pid.kp), 50, button);
}

char* surface_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%ld.%03ld", ((int)settings.surface_pid.kp / SETTINGS_PID_MULTIPLIER), __abs((int)settings.surface_pid.kp % SETTINGS_PID_MULTIPLIER));
	return value;
}


void surface_ki_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.surface_pid.ki), 50, button);
}

char* surface_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%ld.%03ld", ((int)settings.surface_pid.ki / SETTINGS_PID_MULTIPLIER), __abs((int)settings.surface_pid.ki % SETTINGS_PID_MULTIPLIER));
	return value;
}


void surface_kd_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.surface_pid.kd), 50, button);
}

char* surface_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%ld.%03ld", ((int)settings.surface_pid.kd / SETTINGS_PID_MULTIPLIER), __abs((int)settings.surface_pid.kd % SETTINGS_PID_MULTIPLIER));
	return value;
}


void surface_sampling_callback::click(uint16_t button)
{
	callback_click<uint32_t>(&(settings.surface_pid.sampling), 50, button);
}

char* surface_sampling_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.surface_pid.sampling);
	return value;
}




void bigsky_kp_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.bigsky_pid.kp), 50, button);
}

char* bigsky_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%ld.%03ld", ((int)settings.bigsky_pid.kp / SETTINGS_PID_MULTIPLIER), __abs((int)settings.bigsky_pid.kp % SETTINGS_PID_MULTIPLIER));
	return value;
}


void bigsky_ki_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.bigsky_pid.ki), 50, button);
}

char* bigsky_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%ld.%03ld", ((int)settings.bigsky_pid.ki / SETTINGS_PID_MULTIPLIER), __abs((int)settings.bigsky_pid.ki % SETTINGS_PID_MULTIPLIER));
	return value;
}


void bigsky_kd_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.bigsky_pid.kd), 50, button);
}

char* bigsky_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%ld.%03ld", ((int)settings.bigsky_pid.kd / SETTINGS_PID_MULTIPLIER), __abs((int)settings.bigsky_pid.kd % SETTINGS_PID_MULTIPLIER));
	return value;
}


void ground_sampling_callback::click(uint16_t button)
{
	callback_click<uint32_t>(&(settings.bigsky_pid.sampling), 50, button);
}

char* ground_sampling_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.bigsky_pid.sampling);
	return value;
}




void string_kp_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.string_pid.kp), 50, button);
}

char* string_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%ld.%03ld", ((int)settings.string_pid.kp / SETTINGS_PID_MULTIPLIER), __abs((int)settings.string_pid.kp % SETTINGS_PID_MULTIPLIER));
	return value;
}


void string_ki_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.string_pid.ki), 50, button);
}

char* string_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%ld.%03ld", ((int)settings.string_pid.ki / SETTINGS_PID_MULTIPLIER), __abs((int)settings.string_pid.ki % SETTINGS_PID_MULTIPLIER));
	return value;
}


void string_kd_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.string_pid.kd), 50, button);
}

char* string_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%ld.%03ld", ((int)settings.string_pid.kd / SETTINGS_PID_MULTIPLIER), __abs((int)settings.string_pid.kd % SETTINGS_PID_MULTIPLIER));
	return value;
}


void string_sampling_callback::click(uint16_t button)
{
	callback_click<uint32_t>(&(settings.string_pid.sampling), 50, button);
}

char* string_sampling_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.string_pid.sampling);
	return value;
}

