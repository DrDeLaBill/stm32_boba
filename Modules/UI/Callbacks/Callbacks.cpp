/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Callbacks.h"

#include <cstdio>

#include "main.h"
#include "soul.h"
#include "settings.h"
#include "translate.h"


#define PID_STEP      (0.05f)
#define SAMPLING_STEP (50)


void version_callback::click(uint16_t) {}
char* version_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "v%d.%d.%d", DEVICE_VER1, DEVICE_VER2, DEVICE_VER3);
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


void max_pid_time_callback::click(uint16_t button)
{
	const int16_t STEP = 100;
	if (settings.max_pid_time < STEP && button == BTN_DOWN_Pin) {
		settings.max_pid_time = STEP;
		return;
	}
	int16_t tmp = settings.max_pid_time;
	callback_click<int16_t>(&tmp, STEP, button);
	settings.max_pid_time = tmp;
}
char* max_pid_time_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.max_pid_time);
	return value;
}


void surface_kp_callback::click(uint16_t button)
{
	float tmp = settings.surface.kp;
	callback_click<float>(&tmp, PID_STEP, button);
	settings.surface.kp = tmp;
}
char* surface_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.surface.kp), __abs((int)(settings.surface.kp * SETTINGS_PID_MULTIPLIER) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void surface_ki_callback::click(uint16_t button)
{
	float tmp = settings.surface.ki;
	callback_click<float>(&tmp, PID_STEP, button);
	settings.surface.ki = tmp;
}
char* surface_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.surface.ki), __abs((int)(settings.surface.ki * SETTINGS_PID_MULTIPLIER) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void surface_kd_callback::click(uint16_t button)
{
	float tmp = settings.surface.kd;
	callback_click<float>(&tmp, PID_STEP, button);
	settings.surface.kd = tmp;
}
char* surface_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.surface.kd), __abs((int)(settings.surface.kd * SETTINGS_PID_MULTIPLIER) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void surface_sampling_callback::click(uint16_t button)
{
	if (settings.surface.sampling < SAMPLING_STEP && button == BTN_DOWN_Pin) {
		settings.surface.sampling = SAMPLING_STEP;
		return;
	}
	uint32_t tmp = settings.surface.sampling;
	callback_click<uint32_t>(&tmp, SAMPLING_STEP, button);
	settings.surface.sampling = tmp;
}
char* surface_sampling_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.surface.sampling);
	return value;
}




void bigski_kp_callback::click(uint16_t button)
{
	float tmp = settings.bigski.kp;
	callback_click<float>(&tmp, PID_STEP, button);
	settings.bigski.kp = tmp;
}
char* bigski_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.bigski.kp), __abs((int)(settings.bigski.kp * SETTINGS_PID_MULTIPLIER) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void bigski_ki_callback::click(uint16_t button)
{
	float tmp = settings.bigski.ki;
	callback_click<float>(&tmp, PID_STEP, button);
	settings.bigski.ki = tmp;
}
char* bigski_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.bigski.ki), __abs((int)(settings.bigski.ki * SETTINGS_PID_MULTIPLIER) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void bigski_kd_callback::click(uint16_t button)
{
	float tmp = settings.bigski.kd;
	callback_click<float>(&tmp, PID_STEP, button);
	settings.bigski.kd = tmp;
}
char* bigski_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.bigski.kd), __abs((int)(settings.bigski.kd * SETTINGS_PID_MULTIPLIER) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void bigski_sampling_callback::click(uint16_t button)
{
	if (settings.bigski.sampling < SAMPLING_STEP && button == BTN_DOWN_Pin) {
		settings.bigski.sampling = SAMPLING_STEP;
		return;
	}
	uint32_t tmp = settings.bigski.sampling;
	callback_click<uint32_t>(&tmp, SAMPLING_STEP, button);
	settings.bigski.sampling = tmp;
}

char* bigski_sampling_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.bigski.sampling);
	return value;
}




void string_kp_callback::click(uint16_t button)
{
	float tmp = settings.string.kp;
	callback_click<float>(&tmp, PID_STEP, button);
	settings.string.kp = tmp;
}
char* string_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.string.kp), __abs((int)(settings.string.kp * SETTINGS_PID_MULTIPLIER) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void string_ki_callback::click(uint16_t button)
{
	float tmp = settings.string.ki;
	callback_click<float>(&tmp, PID_STEP, button);
	settings.string.ki = tmp;
}
char* string_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.string.ki), __abs((int)(settings.string.ki * SETTINGS_PID_MULTIPLIER) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void string_kd_callback::click(uint16_t button)
{
	float tmp = settings.string.kd;
	callback_click<float>(&tmp, PID_STEP, button);
	settings.string.kd = tmp;
}
char* string_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.string.kd), __abs((int)(settings.string.kd * SETTINGS_PID_MULTIPLIER) % SETTINGS_PID_MULTIPLIER));
	return value;
}


void string_sampling_callback::click(uint16_t button)
{
	if (settings.string.sampling < SAMPLING_STEP && button == BTN_DOWN_Pin) {
		settings.string.sampling = SAMPLING_STEP;
		return;
	}
	uint32_t tmp = settings.string.sampling;
	callback_click<uint32_t>(&tmp, SAMPLING_STEP, button);
	settings.string.sampling = tmp;
}
char* string_sampling_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.string.sampling);
	return value;
}

