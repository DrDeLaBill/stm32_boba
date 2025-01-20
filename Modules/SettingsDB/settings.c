/* Copyright © 2023 Georgy E. All rights reserved. */

#include "settings.h"

#include <string.h>
#include <stdbool.h>

#include "glog.h"
#include "gutils.h"
#include "gsystem.h"
#include "hal_defs.h"
#include "translate.h"


static const char SETTINGS_TAG[] = "STNG";


const uint8_t SENSITIVITY[SETTINGS_BANDS_COUNT] = {
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10
};

const uint32_t SENSITIVITY_DELAY_MS[__arr_len(SENSITIVITY)] = {
	500,
	450,
	400,
	350,
	300,
	350,
	300,
	250,
	200,
	150,
};

const uint16_t DEAD_BANDS_MMx10[__arr_len(SENSITIVITY)] = {
	50,
	40,
	36,
	34,
	30,
	24,
	20,
	20, //16,
	20, //12,
	20, //10
};

const uint16_t PROP_BANDS_MMx10[__arr_len(SENSITIVITY)] = {
	360, // 180,
	160,
	140,
	120,
	100,
	80,
	60,
	50,
	40,
	30
};

const uint16_t ANGLE_DEAD_BANDS[__arr_len(SENSITIVITY)] = {
	5,
	5,
	5,
	4,
	4,
	3,
	3,
	2,
	2,
	1
};

const uint16_t ANGLE_PROP_BANDS[__arr_len(SENSITIVITY)] = {
	45,
	40,
	38,
	35,
	34,
	32,
	30,
	25,
	15,
	10
};


settings_t settings = { 0 };


settings_t* settings_get()
{
	return &settings;
}

void settings_set(settings_t* other)
{
	memcpy((uint8_t*)&settings, (uint8_t*)other, sizeof(settings));
	if (!settings_check(&settings)) {
		settings_repair(&settings);
	}
}

void settings_reset(settings_t* other)
{
	printTagLog(SETTINGS_TAG, "Reset settings");
	other->bedacode = BEDACODE;
	other->dv_type  = DEVICE_TYPE;
	other->fw_id    = FW_VERSION;
	other->language = ENGLISH;

	other->surface_snstv        = 0;
	other->surface_delay        = SETTNNGS_WORK_DELAY_DEFAULT_S;
	other->surface_target       = 0;
	other->surface_reg_wind_x10 = 0;

	other->string_snstv        = 0;
	other->string_delay        = SETTNNGS_WORK_DELAY_DEFAULT_S;
	other->string_target       = 0;
	other->string_reg_wind_x10 = 0;

	other->bigski_snstv = 0;
	other->bigski_delay = SETTNNGS_WORK_DELAY_DEFAULT_S;
	memset((void*)other->bigski_target, 0, sizeof(other->bigski_target));
	other->bigski_reg_wind_x10 = 0;

	other->angle_snstv = 0;
	other->angle_delay = SETTNNGS_WORK_DELAY_DEFAULT_S;
	other->angle_target = 0;

	set_status(NEED_SAVE_SETTINGS);
	reset_status(NEED_RESET_SENSOR);
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
	if (other->fw_id != FW_VERSION) {
		return false;
	}
	if (!IS_LANGUAGE(other->language)) {
		return false;
	}
	uint16_t s_max = __arr_len(SENSITIVITY) - 1;
	if (other->surface_snstv > s_max) {
		return false;
	}
	if (other->string_snstv > s_max) {
		return false;
	}
	if (other->bigski_snstv > s_max) {
		return false;
	}
	if (other->angle_snstv > s_max) {
		return false;
	}
	return true;
}

void settings_repair(settings_t* other)
{
	printTagLog(SETTINGS_TAG, "Repair settings");

	set_status(NEED_SAVE_SETTINGS);

	if (!settings_check(other)) {
		printTagLog(SETTINGS_TAG, "Unable to repair settings");
		settings_reset(other);
	}
}

void settings_show()
{
	gprint("\n");
	printPretty("####################SETTINGS####################\n");
	printPretty("Device type: %u\n", settings.dv_type);
	printPretty("Firmware v%u\n",    settings.fw_id);

    printPretty("------------------------------------------------\n");
	printPretty("Language: %s\n", settings.language == RUSSIAN ? "RUSSIAN" : "ENGLISH"); // TODO
    printPretty("------------------SURFACE MODE------------------\n");
	printPretty("Sensitivity:       %u\n", SENSITIVITY[settings.surface_snstv]);
	printPretty("Dead band:         %u\n", DEAD_BANDS_MMx10[settings.surface_snstv]);
	printPretty("Prop band:         %u\n", PROP_BANDS_MMx10[settings.surface_snstv]);
	printPretty("Sensitivity delay: %lu ms\n", SENSITIVITY_DELAY_MS[settings.surface_snstv]);
	printPretty("Work delay:        %u s\n", settings.surface_delay);
	printPretty("Last target:       %d\n", settings.surface_target);
	printPretty("Regulation window: %u.%u\n", settings.surface_reg_wind_x10 / 10, __abs(settings.surface_reg_wind_x10) % 10);
    printPretty("------------------STRING  MODE------------------\n");
	printPretty("Sensitivity:       %u\n", SENSITIVITY[settings.string_snstv]);
	printPretty("Dead band:         %u\n", DEAD_BANDS_MMx10[settings.string_snstv]);
	printPretty("Prop band:         %u\n", PROP_BANDS_MMx10[settings.string_snstv]);
	printPretty("Sensitivity delay: %lu ms\n", SENSITIVITY_DELAY_MS[settings.string_snstv]);
	printPretty("Work delay:        %u s\n", settings.string_delay);
	printPretty("Last target:       %d\n", settings.string_target);
	printPretty("Regulation window: %u.%u\n", settings.string_reg_wind_x10 / 10, __abs(settings.string_reg_wind_x10) % 10);
    printPretty("------------------BIGSKI  MODE------------------\n");
	printPretty("Sensitivity:       %u\n", SENSITIVITY[settings.bigski_snstv]);
	printPretty("Dead band:         %u\n", DEAD_BANDS_MMx10[settings.bigski_snstv]);
	printPretty("Prop band:         %u\n", PROP_BANDS_MMx10[settings.bigski_snstv]);
	printPretty("Sensitivity delay: %lu ms\n", SENSITIVITY_DELAY_MS[settings.bigski_snstv]);
	printPretty("Work delay:        %u s\n", settings.bigski_delay);
	for (unsigned i = 0; i < __arr_len(settings.bigski_target); i++) {
		printPretty("Last target[%u]:    %d\n", i, settings.bigski_target[i]);
	}
	printPretty("Regulation window: %u.%u\n", settings.bigski_reg_wind_x10 / 10, __abs(settings.bigski_reg_wind_x10) % 10);
    printPretty("------------------ANGLE   MODE------------------\n");
	printPretty("Sensitivity:       %u\n", SENSITIVITY[settings.angle_snstv]);
	printPretty("Dead band:         %u\n", ANGLE_DEAD_BANDS[settings.angle_snstv]);
	printPretty("Prop band:         %u\n", ANGLE_PROP_BANDS[settings.angle_snstv]);
	printPretty("Sensitivity delay: %lu ms\n", SENSITIVITY_DELAY_MS[settings.angle_snstv]);
	printPretty("Work delay:        %u s\n", settings.angle_delay);
	printPretty("Last target:       %d\n", settings.angle_target);
    printPretty("####################SETTINGS####################\n\n");
}
