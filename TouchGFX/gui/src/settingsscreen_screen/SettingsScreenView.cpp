#include <gui/settingsscreen_screen/SettingsScreenView.hpp>

#ifdef SIMULATOR
#else
#   include "main.h"
#   include "sensor.h"
#   include "gsystem.h"
#   include "settings.h"
#endif

#ifdef SIMULATOR
SettingsScreenView::SettingsScreenView():
	settingsPage(0), sensitivity_idx(1), regulation_mm(0)
#else
SettingsScreenView::SettingsScreenView():
	settingsPage(0), sensitivity_idx(0), regulation_mm(0)
#endif
{

}

void SettingsScreenView::setupScreen()
{
    SettingsScreenViewBase::setupScreen();
}

void SettingsScreenView::tearDownScreen()
{
    SettingsScreenViewBase::tearDownScreen();
}

void SettingsScreenView::begin()
{
#ifdef SIMULATOR
#else
	sensitivity_idx = get_sensor_mode_sensitive_index();
	regulation_mm   = get_sensor_mode_regulation_mm();
#endif
}

void SettingsScreenView::update()
{
#ifdef SIMULATOR
#else
	if (system_button_clicked(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin)) {
		goBackAndSave();
		return;
	}

	const unsigned SETTINGS_CNT = 2;
	if (system_button_clicked(BTN_F1_GPIO_Port, BTN_F1_Pin)) {
		settingsPage = (settingsPage > 0) ? settingsPage - 1 : 0;
	}
	if (system_button_clicked(BTN_F2_GPIO_Port, BTN_F2_Pin)) {
		settingsPage = (settingsPage < SETTINGS_CNT - 1) ? settingsPage + 1 : SETTINGS_CNT - 1;
	}

	sensitivityContainer.setVisible(false);
	regulationContainer.setVisible(false);
	unsigned min = 0;
	unsigned max = 0;
	int width2   = 0;
	int x2       = 0;
	switch (settingsPage) {
	case 0:
		min = 0;
	    max = __arr_len(SENSITIVITY);
		if (system_button_clicked(BTN_UP_GPIO_Port, BTN_UP_Pin)) {
			sensitivity_idx = (sensitivity_idx < max) ? sensitivity_idx + 1 : max;
		}
		if (system_button_clicked(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin)) {
			sensitivity_idx = (sensitivity_idx > min) ? sensitivity_idx - 1 : min;
		}
		Unicode::snprintf(
			sensitivityValueBuffer,
			SENSITIVITYVALUE_SIZE - 1,
			"%u",
			SENSITIVITY[sensitivity_idx]
		);
		sensitivityValue.resizeToCurrentText();
		width2 = sensitivityValue.getTextWidth();
		x2     = background.getWidth() / 2 - width2 / 2;
		sensitivityValue.setX(x2);
		sensitivityValue.invalidate();
		sensitivityBackground.invalidate();
		sensitivityContainer.setVisible(true);
		sensitivityContainer.invalidate();
		break;
	case 1:
		min = 0;
	    max = SETTINGS_REGULATION_MM_MAX;
		if (system_button_clicked(BTN_UP_GPIO_Port, BTN_UP_Pin)) {
			regulation_mm = (regulation_mm < max) ? regulation_mm + 1 : max;
		}
		if (system_button_clicked(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin)) {
			regulation_mm = (regulation_mm > min) ? regulation_mm - 1 : min;
		}
		Unicode::snprintf(
			regulationValueBuffer,
			REGULATIONVALUE_SIZE - 1,
			"%u.%u",
			regulation_mm / 10,
			__abs(regulation_mm) % 10
		);
		regulationValue.resizeToCurrentText();
		width2 = regulationValue.getTextWidth();
		x2     = background.getWidth() / 2 - width2 / 2;
		regulationValue.setX(x2);
		regulationValue.invalidate();
		regulationBackground.invalidate();
		regulationContainer.setVisible(true);
		regulationContainer.invalidate();
		break;
	default:
		set_error(UI_ERROR);
		break;
	}
#endif
}

void SettingsScreenView::goBackAndSave()
{
#ifdef SIMULATOR
#else
	set_sensor_mode_sensitive(sensitivity_idx);
	set_sensor_mode_regulation(regulation_mm);
	set_status(NEED_SAVE_SETTINGS);
	goToLoadScrean();
#endif
}
