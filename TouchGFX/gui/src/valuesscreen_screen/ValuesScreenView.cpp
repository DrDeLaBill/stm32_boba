#include <gui/valuesscreen_screen/ValuesScreenView.hpp>

#ifndef SIMULATOR
#   include "gutils.h"
#   include "sensor.h"
#   include "gsystem.h"
#endif


ValuesScreenView::ValuesScreenView()
{

}

void ValuesScreenView::setupScreen()
{
    ValuesScreenViewBase::setupScreen();
}

void ValuesScreenView::tearDownScreen()
{
    ValuesScreenViewBase::tearDownScreen();
}

void ValuesScreenView::changeMode()
{
	if (distanceMode.isVisible()) {
		distanceMode.setVisible(false);
		angleMode.setVisible(true);
	} else {
		angleMode.setVisible(false);
		distanceMode.setVisible(true);
	}
	angleMode.invalidate();
	distanceMode.invalidate();
}

void ValuesScreenView::clickLeft()
{
	changeMode();
}

void ValuesScreenView::clickRight()
{
	changeMode();
}

void ValuesScreenView::updateSensorData()
{
#ifdef SIMULATOR
	static int8_t counter = 0;
	counter++;

	touchgfx::Unicode::UnicodeChar* textBuffer;
	touchgfx::TextAreaWithOneWildcard* text;
	touchgfx::Box* backgroundCleaner;
	uint16_t size;
	if (distanceMode.isVisible()) {
		textBuffer        = distanceValueBuffer;
		text              = &distanceValue;
		size              = DISTANCEVALUE_SIZE;
		backgroundCleaner = &backgroundDistanceCleaner;
	} else {
		textBuffer        = angleValueBuffer;
		text              = &angleValue;
		size              = ANGLEVALUE_SIZE;
		backgroundCleaner = &backgroundAngleCleaner;
	}

	int width1 = text->getTextWidth();
	int x1     = text->getX();
	Unicode::snprintf(
		textBuffer,
		size - 1,
		"%d.%d",
		counter / 10,
		0
	);
	text->resizeToCurrentText();
	int width2 = text->getTextWidth();
	int x2     = x1 + width1 - width2;
	text->setX(x2);
	backgroundCleaner->invalidate();
	text->invalidate();
#else
	touchgfx::Unicode::UnicodeChar* textBuffer;
	touchgfx::TextAreaWithOneWildcard* text;
	touchgfx::Box* backgroundCleaner;
	touchgfx::Container* valueBox;
	uint16_t size;
	int value      = 0;
	bool available = false;
	if (distanceMode.isVisible()) {
		valueBox          = &distanceBox;
		textBuffer        = distanceValueBuffer;
		text              = &distanceValue;
		size              = DISTANCEVALUE_SIZE;
		backgroundCleaner = &backgroundDistanceCleaner;
		available         = sensor_distance_available();
		value             = get_sensor_mode_value(SENSOR_MODE_SURFACE);
	} else {
		valueBox          = &angleBox;
		textBuffer        = angleValueBuffer;
		text              = &angleValue;
		size              = ANGLEVALUE_SIZE;
		backgroundCleaner = &backgroundAngleCleaner;
		available         = sensor_angle_available();
		value             = get_sensor_mode_value(SENSOR_MODE_ANGLE);
	}

	if (!available) {
		emptyValue.setVisible(true);
		valueBox->setVisible(false);
		valueBox->invalidate();
		return;
	}
	emptyValue.setVisible(false);
	valueBox->setVisible(true);

	int width1 = text->getTextWidth();
	int x1     = text->getX();
	if (value == SENSOR_ERROR) {
		Unicode::snprintf(
			textBuffer,
			size - 1,
			"ERROR"
		);
	} else {
		Unicode::snprintf(
			textBuffer,
			size - 1,
			"%d.%d",
			value / SENSOR_DIV_POINT,
			__abs(value % SENSOR_DIV_POINT)
		);
	}
	text->resizeToCurrentText();
	int width2 = text->getTextWidth();
	int x2     = x1 + width1 - width2;
	text->setX(x2);
	backgroundCleaner->invalidate();
	emptyValue.invalidate();
	valueBox->invalidate();
	text->invalidate();
#endif
}
