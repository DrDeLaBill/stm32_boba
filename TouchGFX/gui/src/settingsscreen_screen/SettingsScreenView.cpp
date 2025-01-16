#include <gui/settingsscreen_screen/SettingsScreenView.hpp>

#ifdef SIMULATOR
#else
#   include "main.h"
#   include "sensor.h"
#   include "gsystem.h"
#   include "settings.h"
#endif

#ifdef SIMULATOR
SettingsScreenView::SettingsScreenView(): sensitivity(1)
#else
SettingsScreenView::SettingsScreenView(): sensitivity(get_sensor_mode_sensitive())
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
	sensitivity = get_sensor_mode_sensitive();
#endif
}

void SettingsScreenView::update()
{
#ifdef SIMULATOR
#else
	unsigned min = 1;
	unsigned max = __arr_len(SENSITIVITY);

	bool changed = false;
	if (system_button_clicked(BTN_UP_GPIO_Port, BTN_UP_Pin)) {
		sensitivity = (sensitivity < max) ? sensitivity + 1 : max;
		changed = true;
	}
	if (system_button_clicked(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin)) {
		sensitivity = (sensitivity > min) ? sensitivity - 1 : min;
		changed = true;
	}

	if (!changed) {
		return;
	}

	Unicode::snprintf(
		sensitivityValueBuffer,
		SENSITIVITYVALUE_SIZE - 1,
		"%u",
		sensitivity
	);
	sensitivityValue.resizeToCurrentText();
	int width2 = sensitivityValue.getTextWidth();
	int x2     = background.getWidth() / 2 - width2 / 2;
	sensitivityValue.setX(x2);
	sensitivityValue.invalidate();
	sensitivityBackground.invalidate();
#endif
}

void SettingsScreenView::goBackAndSave()
{
#ifdef SIMULATOR
#else
	set_sensor_mode_sensitive(sensitivity);
	set_status(NEED_SAVE_SETTINGS);
	goToLoadScrean();
#endif
}
