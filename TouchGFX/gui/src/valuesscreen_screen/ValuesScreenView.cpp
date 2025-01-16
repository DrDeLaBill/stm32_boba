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
	uint16_t size;
	switch (get_sensor_mode()) {
	case SENSOR_MODE_STRING:

	case SENSOR_MODE_SURFACE:
	case SENSOR_MODE_BIGSKI:
		distanceMode.setVisible(true);
		angleMode.setVisible(false);
		textBuffer        = distanceValueBuffer;
		text              = &distanceValue;
		size              = DISTANCEVALUE_SIZE;
		backgroundCleaner = &backgroundDistanceCleaner;
		break;
	case SENSOR_MODE_ANGLE:
		angleMode.setVisible(true);
		distanceMode.setVisible(false);
		textBuffer        = angleValueBuffer;
		text              = &angleValue;
		size              = ANGLEVALUE_SIZE;
		backgroundCleaner = &backgroundAngleCleaner;
		break;
	default:
		set_error(UI_ERROR);
		return;
	}

	int width1 = text->getTextWidth();
	int x1     = text->getX();
	int value  = get_sensor_value();
	Unicode::snprintf(
		textBuffer,
		size - 1,
		"%d.%d",
		value / 10,
		__abs(value % 10)
	);
	text->resizeToCurrentText();
	int width2 = text->getTextWidth();
	int x2     = x1 + width1 - width2;
	text->setX(x2);
	backgroundCleaner->invalidate();
	text->invalidate();
#endif
}
