#include <gui/mainscreen_screen/MainScreenView.hpp>

#ifndef SIMULATOR
#   include "gutils.h"
#   include "sensor.h"
#   include "gsystem.h"
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

	if (sensor_available()) {
		textRelative.setVisible(true);
		textAbsolute.setVisible(true);

		int value  = get_sensor_value();
		int width1 = textRelative.getTextWidth();
		int x1     = textRelative.getX();
		Unicode::snprintf(
			textRelativeBuffer,
			TEXTRELATIVE_SIZE - 1,
			"%d.%d",
			value / 10,
			__abs(value % 10)
		);
		textRelative.resizeToCurrentText();
		int width2 = textRelative.getTextWidth();
		int x2     = x1 + width1 - width2;
		textRelative.setX(x2);
		textRelative.invalidate();

		value  = get_sensor_mode_target(get_sensor_mode());
		width1 = textAbsolute.getTextWidth();
		x1     = textAbsolute.getX();
		Unicode::snprintf(
			textAbsoluteBuffer,
			TEXTABSOLUTE_SIZE - 1,
			"%d.%d",
			value / 10,
			__abs(value % 10)
		);
		textAbsolute.resizeToCurrentText();
		width2 = textAbsolute.getTextWidth();
		x2     = x1 + width1 - width2;
		textAbsolute.setX(x2);
		textAbsolute.invalidate();
	} else {

	}

	backgroundRelative.invalidate();
#endif
}
