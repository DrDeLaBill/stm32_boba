/* Copyright © 2023 Georgy E. All rights reserved. */

#include "settings.h"

#include <string.h>
#include <stdbool.h>

#include "log.h"
#include "utils.h"
#include "hal_defs.h"


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
	other->sw_id = SW_VERSION;
	other->fw_id = FW_VERSION;
	other->cf_id = CF_VERSION;

	other->last_target = 0;
	other->kp = SETTINGS_DEFUALT_KP;
	other->ki = SETTINGS_DEFUALT_KI;
	other->kd = SETTINGS_DEFUALT_KD;
	other->sampling = SETTINGS_DEFAULT_SAMPLING;
}

uint32_t settings_size()
{
	return sizeof(settings_t);
}

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
	printPretty(
		"PID coefficients: Kp=%ld.%ld, Ki=%ld.%ld, Kd=%ld.%ld\n",
		((int)settings.kp), __abs(((int)(settings.kp * 100)) % 100),
		((int)settings.ki), __abs(((int)(settings.ki * 100)) % 100),
		((int)settings.kd), __abs(((int)(settings.kd * 100)) % 100)
	);
	printPretty("PID sampling: %lu ms\n", settings.sampling);
    printPretty("####################SETTINGS####################\n\n");
}
