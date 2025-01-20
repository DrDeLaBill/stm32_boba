#include <gui/sensorlist_screen/SensorListView.hpp>

#ifndef SIMULATOR
#   include "main.h"
#   include "gutils.h"
#   include "sensor.h"
#   include "gsystem.h"
#endif


SensorListView::SensorListView(): listCnt(0)
{

}

void SensorListView::setupScreen()
{
    SensorListViewBase::setupScreen();
}

void SensorListView::tearDownScreen()
{
    SensorListViewBase::tearDownScreen();
}

void SensorListView::updateValues()
{
#ifdef SIMULATOR
	static int8_t counter = 0;
	counter++;

	int width1 = angleValue.getTextWidth();
	int x1     = angleValue.getX();
	Unicode::snprintf(
		angleValueBuffer,
		ANGLEVALUE_SIZE - 1,
		"%d.%d",
		counter / 10,
		0
	);
	angleValue.resizeToCurrentText();
	int width2 = angleValue.getTextWidth();
	int x2     = x1 + width1 - width2;
	angleValue.setX(x2);
	if (backgroundAngle.isVisible()) {
		angleBack.setColor(backgroundAngle.getColor());
	} else {
		angleBack.setColor(background.getColor());
	}
	angleBack.invalidate();
	angleValue.invalidate();


	width1 = distanceValue.getTextWidth();
	x1     = distanceValue.getX();
	Unicode::snprintf(
		distanceValueBuffer,
		DISTANCEVALUE_SIZE - 1,
		"%d.%d",
		counter / 10,
		0
	);
	distanceValue.resizeToCurrentText();
	width2 = distanceValue.getTextWidth();
	x2     = x1 + width1 - width2;
	distanceValue.setX(x2);
	if (backgroundDistance.isVisible()) {
		distanceBack.setColor(backgroundDistance.getColor());
	} else {
		distanceBack.setColor(background.getColor());
	}
	distanceBack.invalidate();
	distanceValue.invalidate();
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

	visibleDistanceContainer.setVisible(false);
	visibleAngleContainer.setVisible(false);
	visibleContainer3.setVisible(false);
	emptyDistanceContainer.setVisible(true);
	emptyAngleContainer.setVisible(true);
	emptyContainer3.setVisible(true);

	if (sensor_distance_available()) {
		emptyDistanceContainer.setVisible(false);
		visibleDistanceContainer.setVisible(true);

		int value  = get_sensor_mode_value(SENSOR_MODE_SURFACE);
		int width1 = distanceValue.getTextWidth();
		int x1     = distanceValue.getX();
		Unicode::snprintf(
			distanceValueBuffer,
			DISTANCEVALUE_SIZE - 1,
			"%d.%d",
			value / SENSOR_DIV_POINT,
			__abs(value % SENSOR_DIV_POINT)
		);
		distanceValue.resizeToCurrentText();
		int width2 = distanceValue.getTextWidth();
		int x2     = x1 + width1 - width2;
		distanceValue.setX(x2);
		if (backgroundDistance.isVisible()) {
			distanceBack.setColor(backgroundDistance.getColor());
		} else {
			distanceBack.setColor(background.getColor());
		}
		distanceBack.invalidate();
		distanceValue.invalidate();
	}

	if (sensor_angle_available()) {
		emptyAngleContainer.setVisible(false);
		visibleAngleContainer.setVisible(true);

		int value  = get_sensor_mode_value(SENSOR_MODE_ANGLE);
		int width1 = angleValue.getTextWidth();
		int x1     = angleValue.getX();
		if (value == SENSOR_ERROR) {
			Unicode::snprintf(
				angleValueBuffer,
				ANGLEVALUE_SIZE - 1,
				"ERROR"
			);
		} else {
			Unicode::snprintf(
				angleValueBuffer,
				ANGLEVALUE_SIZE - 1,
				"%d.%d",
				value / SENSOR_DIV_POINT,
				__abs(value % SENSOR_DIV_POINT)
			);
		}
		angleValue.resizeToCurrentText();
		int width2 = angleValue.getTextWidth();
		int x2     = x1 + width1 - width2;
		angleValue.setX(x2);
		if (backgroundAngle.isVisible()) {
			angleBack.setColor(backgroundAngle.getColor());
		} else {
			angleBack.setColor(background.getColor());
		}
		angleBack.invalidate();
		angleValue.invalidate();
	}


	const unsigned LIST_SIZE = 3;
	if (system_button_clicked(BTN_UP_GPIO_Port, BTN_UP_Pin)) {
		listCnt = (listCnt > 0) ? listCnt - 1 : 0;
	}
	if (system_button_clicked(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin)) {
		listCnt = (listCnt < LIST_SIZE - 1) ? listCnt + 1 : LIST_SIZE - 1;
	}

	backgroundDistance.setVisible(false);
	backgroundAngle.setVisible(false);
	background3.setVisible(false);
	distanceBack.setColor(background.getColor());
	angleBack.setColor(background.getColor());
	switch (listCnt) {
	case 0:
		backgroundDistance.setVisible(true);
		distanceBack.setColor(backgroundDistance.getColor());
		if (sensor_distance_available() &&
			system_button_clicked(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin)
		) {
			set_sensor_mode(SENSOR_MODE_SURFACE);
			clickEnter();
		}
		break;
	case 1:
		backgroundAngle.setVisible(true);
		angleBack.setColor(backgroundAngle.getColor());
		if (sensor_angle_available() &&
			system_button_clicked(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin)
		) {
			set_sensor_mode(SENSOR_MODE_ANGLE);
			clickEnter();
		}
		break;
	case 2:
		background3.setVisible(true);
		break;
	default:
		set_error(UI_ERROR);
		return;
	}
	backgroundDistance.invalidate();
	backgroundAngle.invalidate();
	distanceBack.invalidate();
	angleBack.invalidate();
	background3.invalidate();
#endif
}
