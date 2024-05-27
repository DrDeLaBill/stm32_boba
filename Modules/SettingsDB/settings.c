/* Copyright © 2023 Georgy E. All rights reserved. */

#include "settings.h"

#include <string.h>
#include <stdbool.h>

#include "log.h"
#include "utils.h"
#include "hal_defs.h"
#include "translate.h"


static const char SETTINGS_TAG[] = "STNG";


settings_t settings = { 0 };


settings_t* settings_get()
{
	return &settings;
}

void settings_set(settings_t* other)
{
	memcpy((uint8_t*)&settings, (uint8_t*)other, sizeof(settings));
}

void settings_reset(settings_t* other)
{
	printTagLog(SETTINGS_TAG, "Reset settings");
	other->dv_type = DEVICE_TYPE;
	other->sw_id   = SW_VERSION;
	other->fw_id   = FW_VERSION;
	other->cf_id   = CF_VERSION;

	other->max_pid_time = SETTINGS_DEFAULT_PID_MAX;
	other->language     = ENGLISH;

	other->surface.kp = SETTINGS_DEFUALT_SURFACE_KP;
	other->surface.ki = SETTINGS_DEFUALT_SURFACE_KI;
	other->surface.kd = SETTINGS_DEFUALT_SURFACE_KD;
	other->surface.sampling = SETTINGS_DEFAULT_SURFACE_SAMPLING;
	other->surface_target = 0;

	other->string.kp = SETTINGS_DEFUALT_STRING_KP;
	other->string.ki = SETTINGS_DEFUALT_STRING_KI;
	other->string.kd = SETTINGS_DEFUALT_STRING_KD;
	other->string.sampling = SETTINGS_DEFAULT_STRING_SAMPLING;
	other->string_target = 0;

	other->bigski.kp = SETTINGS_DEFUALT_GROUND_KP;
	other->bigski.ki = SETTINGS_DEFUALT_GROUND_KI;
	other->bigski.kd = SETTINGS_DEFUALT_GROUND_KD;
	other->bigski.sampling = SETTINGS_DEFAULT_GROUND_SAMPLING;
	memset((void*)other->bigski_target, 0, sizeof(other->bigski_target));
}

uint32_t settings_size()
{
	return sizeof(settings_t);
}

// TODO: version updater (if version older than current -> update settings)
bool settings_check(settings_t* other)
{
	if (other->dv_type != DEVICE_TYPE) {
		return false;
	}
	if (other->sw_id != SW_VERSION) {
		return false;
	}
	if (other->fw_id != FW_VERSION) {
		return false;
	}
	if (!IS_LANGUAGE(other->language)) {
		return false;
	}
	return true;
}

void settings_show()
{
	gprint("\n");
	printPretty("####################SETTINGS####################\n");
	printPretty("Device type: %u\n", settings.dv_type);
	printPretty("Software v%u\n", settings.sw_id);
	printPretty("Firmware v%u\n", settings.fw_id);
	printPretty("Configuration ID: %lu\n", settings.cf_id);

    printPretty("------------------------------------------------\n");
	printPretty("Language: %s\n", settings.language == RUSSIAN ? "RUSSIAN" : "ENGLISH"); // TODO
	printPretty("Max PID output time: %lu ms\n", settings.max_pid_time);
    printPretty("------------------SURFACE MODE------------------\n");
	printPretty(
		"PID coefficients: Kp=%ld.%ld, Ki=%ld.%ld, Kd=%ld.%ld\n",
		((int)settings.surface.kp), __abs(((int)(settings.surface.kp * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER),
		((int)settings.surface.ki), __abs(((int)(settings.surface.ki * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER),
		((int)settings.surface.kd), __abs(((int)(settings.surface.kd * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER)
	);
	printPretty("PID sampling: %lu ms\n", settings.surface.sampling);
	printPretty("Last target: %ld\n", settings.surface_target);
    printPretty("------------------STRING  MODE------------------\n");
	printPretty(
		"PID coefficients: Kp=%ld.%ld, Ki=%ld.%ld, Kd=%ld.%ld\n",
		((int)settings.string.kp), __abs(((int)(settings.string.kp * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER),
		((int)settings.string.ki), __abs(((int)(settings.string.ki * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER),
		((int)settings.string.kd), __abs(((int)(settings.string.kd * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER)
	);
	printPretty("PID sampling: %lu ms\n", settings.string.sampling);
	printPretty("Last target: %ld\n", settings.string_target);
    printPretty("------------------BIGSKI  MODE------------------\n");
	printPretty(
		"PID coefficients: Kp=%ld.%ld, Ki=%ld.%ld, Kd=%ld.%ld\n",
		((int)settings.bigski.kp), __abs(((int)(settings.bigski.kp * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER),
		((int)settings.bigski.ki), __abs(((int)(settings.bigski.ki * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER),
		((int)settings.bigski.kd), __abs(((int)(settings.bigski.kd * SETTINGS_PID_MULTIPLIER)) % SETTINGS_PID_MULTIPLIER)
	);
	printPretty("PID sampling: %lu ms\n", settings.bigski.sampling);
	for (unsigned i = 0; i < __arr_len(settings.bigski_target); i++) {
		printPretty("Last target[%u]: %ld\n", i, settings.bigski_target[i]);
	}
    printPretty("####################SETTINGS####################\n\n");
}
