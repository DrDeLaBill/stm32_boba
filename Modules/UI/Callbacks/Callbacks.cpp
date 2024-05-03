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
	callback_click<float>(&(settings.surface_kp), 0.05f, button);
}

char* surface_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.surface_kp), __abs(((int)(settings.surface_kp * 100)) % 100));
	return value;
}


void surface_ki_callback::click(uint16_t button)
{
	callback_click<float>(&(settings.surface_ki), 0.05f, button);
}

char* surface_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.surface_ki), __abs(((int)(settings.surface_ki * 100)) % 100));
	return value;
}


void surface_kd_callback::click(uint16_t button)
{
	callback_click<float>(&(settings.surface_kd), 0.05f, button);
}

char* surface_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.surface_kd), __abs(((int)(settings.surface_kd * 100)) % 100));
	return value;
}


void surface_sampling_callback::click(uint16_t button)
{
	callback_click<uint32_t>(&(settings.surface_sampling), 50, button);
}

char* surface_sampling_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.surface_sampling);
	return value;
}




void ground_kp_callback::click(uint16_t button)
{
	callback_click<float>(&(settings.ground_kp), 0.05f, button);
}

char* ground_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.ground_kp), __abs(((int)(settings.ground_kp * 100)) % 100));
	return value;
}


void ground_ki_callback::click(uint16_t button)
{
	callback_click<float>(&(settings.ground_ki), 0.05f, button);
}

char* ground_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.ground_ki), __abs(((int)(settings.ground_ki * 100)) % 100));
	return value;
}


void ground_kd_callback::click(uint16_t button)
{
	callback_click<float>(&(settings.ground_kd), 0.05f, button);
}

char* ground_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.ground_kd), __abs(((int)(settings.ground_kd * 100)) % 100));
	return value;
}


void ground_sampling_callback::click(uint16_t button)
{
	callback_click<uint32_t>(&(settings.ground_sampling), 50, button);
}

char* ground_sampling_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.ground_sampling);
	return value;
}




void string_kp_callback::click(uint16_t button)
{
	callback_click<float>(&(settings.string_kp), 0.05f, button);
}

char* string_kp_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.string_kp), __abs(((int)(settings.string_kp * 100)) % 100));
	return value;
}


void string_ki_callback::click(uint16_t button)
{
	callback_click<float>(&(settings.string_ki), 0.05f, button);
}

char* string_ki_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.string_ki), __abs(((int)(settings.string_ki * 100)) % 100));
	return value;
}


void string_kd_callback::click(uint16_t button)
{
	callback_click<float>(&(settings.string_kd), 0.05f, button);
}

char* string_kd_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.string_kd), __abs(((int)(settings.string_kd * 100)) % 100));
	return value;
}


void string_sampling_callback::click(uint16_t button)
{
	callback_click<uint32_t>(&(settings.string_sampling), 50, button);
}

char* string_sampling_callback::value()
{
	static char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%lu ms", settings.string_sampling);
	return value;
}

