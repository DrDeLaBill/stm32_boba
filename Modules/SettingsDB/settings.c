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

	other->last_target = 0;
	other->language    = ENGLISH;

	other->surface_pid.kp = SETTINGS_DEFUALT_SURFACE_KP;
	other->surface_pid.ki = SETTINGS_DEFUALT_SURFACE_KI;
	other->surface_pid.kd = SETTINGS_DEFUALT_SURFACE_KD;
	other->surface_pid.sampling = SETTINGS_DEFAULT_SURFACE_SAMPLING;

	other->ground_pid.kp = SETTINGS_DEFUALT_GROUND_KP;
	other->ground_pid.ki = SETTINGS_DEFUALT_GROUND_KI;
	other->ground_pid.kd = SETTINGS_DEFUALT_GROUND_KD;
	other->ground_pid.sampling = SETTINGS_DEFAULT_GROUND_SAMPLING;

	other->string_pid.kp = SETTINGS_DEFUALT_STRING_KP;
	other->string_pid.ki = SETTINGS_DEFUALT_STRING_KI;
	other->string_pid.kd = SETTINGS_DEFUALT_STRING_KD;
	other->string_pid.sampling = SETTINGS_DEFAULT_STRING_SAMPLING;
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
	printPretty("Last valve target: %ld\n", settings.last_target);
	printPretty("Language: %s\n", settings.language == RUSSIAN ? "RUSSIAN" : "ENGLISH"); // TODO
    printPretty("------------------SURFACE MODE------------------\n");
	printPretty(
		"PID coefficients: Kp=%ld.%ld, Ki=%ld.%ld, Kd=%ld.%ld\n",
		((int)settings.surface_pid.kp), __abs(((int)(settings.surface_pid.kp * 100)) % 100),
		((int)settings.surface_pid.ki), __abs(((int)(settings.surface_pid.ki * 100)) % 100),
		((int)settings.surface_pid.kd), __abs(((int)(settings.surface_pid.kd * 100)) % 100)
	);
	printPretty("PID sampling: %lu ms\n", settings.surface_pid.sampling);
    printPretty("------------------GROUND  MODE------------------\n");
	printPretty(
		"PID coefficients: Kp=%ld.%ld, Ki=%ld.%ld, Kd=%ld.%ld\n",
		((int)settings.ground_pid.kp), __abs(((int)(settings.ground_pid.kp * 100)) % 100),
		((int)settings.ground_pid.ki), __abs(((int)(settings.ground_pid.ki * 100)) % 100),
		((int)settings.ground_pid.kd), __abs(((int)(settings.ground_pid.kd * 100)) % 100)
	);
	printPretty("PID sampling: %lu ms\n", settings.ground_pid.sampling);
    printPretty("------------------STRING  MODE------------------\n");
	printPretty(
		"PID coefficients: Kp=%ld.%ld, Ki=%ld.%ld, Kd=%ld.%ld\n",
		((int)settings.string_pid.kp), __abs(((int)(settings.string_pid.kp * 100)) % 100),
		((int)settings.string_pid.ki), __abs(((int)(settings.string_pid.ki * 100)) % 100),
		((int)settings.string_pid.kd), __abs(((int)(settings.string_pid.kd * 100)) % 100)
	);
	printPretty("PID sampling: %lu ms\n", settings.string_pid.sampling);
    printPretty("####################SETTINGS####################\n\n");
}
