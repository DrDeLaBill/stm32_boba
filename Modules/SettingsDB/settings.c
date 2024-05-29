/* Copyright © 2023 Georgy E. All rights reserved. */

#include "settings.h"

#include <string.h>
#include <stdbool.h>

#include "log.h"
#include "soul.h"
#include "utils.h"
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

const uint16_t DEAD_BANDS_MMx10[__arr_len(SENSITIVITY)] = {
	50,
	40,
	36,
	34,
	30,
	24,
	20,
	16,
	12,
	10
};

const uint16_t PROP_BANDS_MMx10[__arr_len(SENSITIVITY)] = {
	180,
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
	other->dv_type = DEVICE_TYPE;
	other->sw_id   = SW_VERSION;
	other->fw_id   = FW_VERSION;
	other->cf_id   = CF_VERSION;

	other->language     = ENGLISH;

	other->surface_snstv = 0;
	other->surface_target = 0;

	other->string_snstv = 0;
	other->string_target = 0;

	other->bigski_snstv = 0;
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
	uint16_t s_min = 0;
	uint16_t s_max = __arr_len(SENSITIVITY) - 1;
	if (s_min > settings.surface_snstv || settings.surface_snstv > s_max) {
		return false;
	}
	if (s_min > settings.string_snstv || settings.string_snstv > s_max) {
		return false;
	}
	if (s_min > settings.bigski_snstv || settings.bigski_snstv > s_max) {
		return false;
	}
	return true;
}

void settings_repair(settings_t* other)
{
	printTagLog(SETTINGS_TAG, "Repair settings");

	set_status(NEED_SAVE_SETTINGS);

	if (other->fw_id != FW_VERSION) {
		other->fw_id = FW_VERSION;
	}

	if (!settings_check(other)) {
		settings_reset(other);
	}
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
    printPretty("------------------SURFACE MODE------------------\n");
	printPretty("Sensitivity: %u\n", SENSITIVITY[settings.surface_snstv]);
	printPretty("Dead band: %u\n", DEAD_BANDS_MMx10[settings.surface_snstv]);
	printPretty("Prop band: %u\n", PROP_BANDS_MMx10[settings.surface_snstv]);
	printPretty("Last target: %d\n", settings.surface_target);
    printPretty("------------------STRING  MODE------------------\n");
	printPretty("Sensitivity: %u\n", SENSITIVITY[settings.string_snstv]);
	printPretty("Dead band: %u\n", DEAD_BANDS_MMx10[settings.string_snstv]);
	printPretty("Prop band: %u\n", PROP_BANDS_MMx10[settings.string_snstv]);
	printPretty("Last target: %d\n", settings.string_target);
    printPretty("------------------BIGSKI  MODE------------------\n");
	printPretty("Sensitivity: %u\n", SENSITIVITY[settings.bigski_snstv]);
	printPretty("Dead band: %u\n", DEAD_BANDS_MMx10[settings.bigski_snstv]);
	printPretty("Prop band: %u\n", PROP_BANDS_MMx10[settings.bigski_snstv]);
	for (unsigned i = 0; i < __arr_len(settings.bigski_target); i++) {
		printPretty("Last target[%u]: %d\n", i, settings.bigski_target[i]);
	}
    printPretty("####################SETTINGS####################\n\n");
}
