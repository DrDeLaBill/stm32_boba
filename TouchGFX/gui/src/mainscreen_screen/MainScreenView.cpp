#include <gui/mainscreen_screen/MainScreenView.hpp>

#ifndef SIMULATOR
#   include "main.h"
#   include "gutils.h"
#   include "sensor.h"
#   include "gsystem.h"

#   include "App.hpp"
#endif


MainScreenView::MainScreenView()
{

}

void MainScreenView::setupScreen()
{
    MainScreenViewBase::setupScreen();
}

void MainScreenView::tearDownScreen()
{
    MainScreenViewBase::tearDownScreen();
}

void MainScreenView::updateSensorData()
{
#ifdef SIMULATOR
	static int8_t counter = 0;
	counter++;

	int width1 = textRelative.getTextWidth();
	int x1     = textRelative.getX();
	Unicode::snprintf(
		textRelativeBuffer,
		TEXTRELATIVE_SIZE - 1,
		"%d.%d",
		counter / 10,
		0
	);
	textRelative.resizeToCurrentText();
	int width2 = textRelative.getTextWidth();
	int x2     = x1 + width1 - width2;
	textRelative.setX(x2);
	textRelative.invalidate();

	width1 = textAbsolute.getTextWidth();
	x1     = textAbsolute.getX();
	Unicode::snprintf(
		textAbsoluteBuffer,
		TEXTABSOLUTE_SIZE - 1,
		"%d.%d",
		counter / 10,
		0
	);
	textAbsolute.resizeToCurrentText();
	width2 = textAbsolute.getTextWidth();
	x2     = x1 + width1 - width2;
	textAbsolute.setX(x2);
	textAbsolute.invalidate();

	backgroundRelative.invalidate();
#else

	distanceMode.setVisible(false);
	angleMode.setVisible(false);

	distanceModeImg.setVisible(false);
	stringModeImg.setVisible(false);

	switch (get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		distanceMode.setVisible(true);
		distanceModeImg.setVisible(true);
		break;
	case SENSOR_MODE_STRING:
		distanceMode.setVisible(true);
		stringModeImg.setVisible(true);
		break;
	case SENSOR_MODE_BIGSKI:
		distanceMode.setVisible(true);
		break;
	case SENSOR_MODE_ANGLE:
		angleMode.setVisible(true);
		break;
	default:
		set_error(UI_ERROR);
		return;
	}
	distanceMode.invalidate();
	angleMode.invalidate();
	distanceModeImg.invalidate();
	stringModeImg.invalidate();

	if (system_button_pressed(BTN_UP_GPIO_Port, BTN_UP_Pin)) {
		set_status(MANUAL_NEED_VALVE_UP);
	} else {
		reset_status(MANUAL_NEED_VALVE_UP);
	}
	if (system_button_pressed(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin)) {
		set_status(MANUAL_NEED_VALVE_DOWN);
	} else {
		reset_status(MANUAL_NEED_VALVE_DOWN);
	}

	backgroundRelative.invalidate();
	if (sensor_available()) {
		textRelative.setVisible(true);
		textAbsolute.setVisible(true);
		emptyRealtive.setVisible(false);
		emptyAbsolute.setVisible(false);

		int value  = get_sensor_value();
		int width1 = textRelative.getTextWidth();
		int x1     = textRelative.getX();
		if (value == SENSOR_ERROR) {
			Unicode::snprintf(
				textRelativeBuffer,
				TEXTRELATIVE_SIZE - 1,
				"ERROR"
			);
		} else {
			Unicode::snprintf(
				textRelativeBuffer,
				TEXTRELATIVE_SIZE - 1,
				"%d.%d",
				value / SENSOR_DIV_POINT,
				__abs(value % SENSOR_DIV_POINT)
			);
		}
		textRelative.resizeToCurrentText();
		int width2 = textRelative.getTextWidth();
		int x2     = x1 + width1 - width2;
		textRelative.setX(x2);
		textRelative.invalidate();

		if (value != SENSOR_ERROR) {
			value += get_sensor_mode_target(get_sensor_mode());
		}
		width1 = textAbsolute.getTextWidth();
		x1     = textAbsolute.getX();
		if (value == SENSOR_ERROR) {
			Unicode::snprintf(
				textAbsoluteBuffer,
				TEXTABSOLUTE_SIZE - 1,
				""
			);
		} else {
			Unicode::snprintf(
				textAbsoluteBuffer,
				TEXTABSOLUTE_SIZE - 1,
				"%d.%d",
				value / SENSOR_DIV_POINT,
				__abs(value % SENSOR_DIV_POINT)
			);
		}
		textAbsolute.resizeToCurrentText();
		width2 = textAbsolute.getTextWidth();
		x2     = x1 + width1 - width2;
		textAbsolute.setX(x2);
		textAbsolute.invalidate();

		value  = get_sensor_value();
		if (angleMode.isVisible()) {
			degreeRight.setVisible(value > get_sensor_mode_sensitive());
			degreeLeft.setVisible(-value > get_sensor_mode_sensitive());
			degreeLeft.invalidate();
			degreeRight.invalidate();
		}

		if (system_button_clicked(BTN_MODE_GPIO_Port, BTN_MODE_Pin)) {
			App::setAppMode(App::getAppMode() == APP_MODE_AUTO ? APP_MODE_MANUAL : APP_MODE_AUTO);
		}

		bool autoMode = App::getAppMode() == APP_MODE_AUTO;
		autoText.setVisible(autoMode);
		autoText.invalidate();
		if (autoMode) {
			return;
		}

		if (system_button_clicked(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin)) {
			save_sensor_mode_target();
			set_status(NEED_SAVE_SETTINGS);
			setLoadScrean();
		}
		if (system_button_holded(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin)) {
			reset_sensor_mode_target();
			set_status(NEED_SAVE_SETTINGS);
			setLoadScrean();
		}
	} else {
		textRelative.setVisible(false);
		textAbsolute.setVisible(false);
		emptyRealtive.setVisible(true);
		emptyAbsolute.setVisible(true);
	}
#endif
}
