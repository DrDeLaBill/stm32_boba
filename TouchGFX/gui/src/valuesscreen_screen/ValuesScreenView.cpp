#include <gui/valuesscreen_screen/ValuesScreenView.hpp>

#ifndef SIMULATOR
#   include "main.h"
#   include "gutils.h"
#   include "sensor.h"
#   include "gsystem.h"
#endif


ValuesScreenView::ValuesScreenView(): listIdx(0)
{

}

void ValuesScreenView::setupScreen()
{
    ValuesScreenViewBase::setupScreen();
	listIdx = 0;
}

void ValuesScreenView::tearDownScreen()
{
    ValuesScreenViewBase::tearDownScreen();
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
	distanceMode.setVisible(false);
	distanceBox.setVisible(false);
	distanceMode.invalidate();
	distanceBox.invalidate();

	angleMode.setVisible(false);
	angleBox.setVisible(false);
	angleBox.invalidate();
	angleBox.invalidate();

	touchgfx::Unicode::UnicodeChar* textBuffer;
	touchgfx::TextAreaWithOneWildcard* text;
	touchgfx::Box* backgroundCleaner;
	touchgfx::Container* container;
	touchgfx::Container* valueBox;
	uint16_t size;
	int value      = 0;
	bool available = false;
	switch (listIdx) {
	case 0:
		container         = &distanceMode;
		valueBox          = &distanceBox;
		textBuffer        = distanceValueBuffer;
		text              = &distanceValue;
		size              = DISTANCEVALUE_SIZE;
		backgroundCleaner = &backgroundDistanceCleaner;
		available         = sensor_distance_available();
		value             = get_sensor_mode_value(SENSOR_MODE_SURFACE);
		break;
	case 1:
		container         = &angleMode;
		valueBox          = &angleBox;
		textBuffer        = angleValueBuffer;
		text              = &angleValue;
		size              = ANGLEVALUE_SIZE;
		backgroundCleaner = &backgroundAngleCleaner;
		available         = sensor_angle_available();
		value             = get_sensor_mode_value(SENSOR_MODE_ANGLE);
		break;
	default:
		set_error(UI_ERROR);
		break;
	}

	const unsigned LIST_SIZE = 2;
	if (system_button_clicked(BTN_F1_GPIO_Port, BTN_F1_Pin)) {
		listIdx = (listIdx > 0) ? listIdx - 1 : 0;
	}
	if (system_button_clicked(BTN_F2_GPIO_Port, BTN_F2_Pin)) {
		listIdx = (listIdx < LIST_SIZE - 1) ? listIdx + 1 : LIST_SIZE - 1;
	}

	container->setVisible(true);
	emptyValue.setVisible(!available);
	valueBox->setVisible(available);
	emptyValue.invalidate();
	valueBox->invalidate();
	if (!available) {
		return;
	}

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
