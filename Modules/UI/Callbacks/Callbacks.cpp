/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Callbacks.h"

#include <cstdio>

#include "main.h"
#include "soul.h"
#include "settings.h"


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
	snprintf(value, sizeof(value), "v%d.%d.%d", DEVICE_VER1, DEVICE_VER2, DEVICE_VER3);
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



void surface_kp_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.surface_pid.kp), 50, button);
}

char* surface_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.surface_pid.kp), __abs(((int)(settings.surface_pid.kp * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void surface_ki_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.surface_pid.ki), 50, button);
}

char* surface_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.surface_pid.ki), __abs(((int)(settings.surface_pid.ki * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void surface_kd_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.surface_pid.kd), 50, button);
}

char* surface_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.surface_pid.kd), __abs(((int)(settings.surface_pid.kd * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER));
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




void ground_kp_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.ground_pid.kp), 50, button);
}

char* ground_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.ground_pid.kp), __abs(((int)(settings.ground_pid.kp * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void ground_ki_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.ground_pid.ki), 50, button);
}

char* ground_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.ground_pid.ki), __abs(((int)(settings.ground_pid.ki * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void ground_kd_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.ground_pid.kd), 50, button);
}

char* ground_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.ground_pid.kd), __abs(((int)(settings.ground_pid.kd * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void ground_sampling_callback::click(uint16_t button)
{
	callback_click<uint32_t>(&(settings.ground_pid.sampling), 50, button);
}

char* ground_sampling_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.ground_pid.sampling);
	return value;
}




void string_kp_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.string_pid.kp), 50, button);
}

char* string_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.string_pid.kp), __abs(((int)(settings.string_pid.kp * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void string_ki_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.string_pid.ki), 50, button);
}

char* string_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.string_pid.ki), __abs(((int)(settings.string_pid.ki * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void string_kd_callback::click(uint16_t button)
{
	callback_click<int32_t>(&(settings.string_pid.kd), 50, button);
}

char* string_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.string_pid.kd), __abs(((int)(settings.string_pid.kd * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER));
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

