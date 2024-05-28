/* Copyright © 2024 Georgy E. All rights reserved. */

#include "App.h"

#include "log.h"
#include "main.h"
#include "soul.h"
#include "sensor.h"
#include "settings.h"
#include "hal_defs.h"


fsm::FiniteStateMachine<App::fsm_table> App::fsm;
uint16_t App::deadBand = 0;
uint16_t App::propBand = 0;
utl::Timer App::sampleTimer(App::SAMPLE_PWM_MS);
utl::Timer App::workTimer(0);
SENSOR_MODE App::sensorMode = SENSOR_MODE_SURFACE;
APP_MODE App::appMode = APP_MODE_MANUAL;
int16_t App::value = 0;


void App::proccess()
{
	fsm.proccess();

	if (get_sensor_mode() == SENSOR_MODE_BIGSKI) {
		if (sensor2AB_available() && sensor2A7_available() && sensor2A8_available()) {
			value = get_sensor_average();
			reset_status(NO_BIGSKI);
		} else {
			set_status(NO_BIGSKI);
		}
	} else {
		value = get_sensor2A7_value();
	}
}

void App::setAppMode(APP_MODE mode)
{
	if (appMode == mode) {
		return;
	}

	if (mode == APP_MODE_AUTO) {
		fsm.push_event(auto_e{});
	}

	if (mode == APP_MODE_MANUAL) {
		fsm.push_event(manual_e{});
	}

	App::appMode = mode;
}

int16_t App::getValue()
{
	return value;
}

APP_MODE App::getAppMode()
{
	return appMode;
}

void App::changeSensorMode(SENSOR_MODE mode)
{
	set_sensor_mode(mode);
	sensorMode = mode;
}

void App::up()
{
	HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_SET);
	reset_status(AUTO_NEED_VALVE_DOWN);
	set_status(AUTO_NEED_VALVE_UP);
}

void App::down()
{
	HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_SET);
	reset_status(AUTO_NEED_VALVE_UP);
	set_status(AUTO_NEED_VALVE_DOWN);
}

void App::stop()
{
	HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_RESET);
	reset_status(AUTO_NEED_VALVE_DOWN);
	reset_status(AUTO_NEED_VALVE_UP);
}

void App::_init_s::operator ()()
{
	stop();

	if (is_status(WAIT_LOAD)) {
		return;
	}

	fsm.push_event(success_e{});
}

void App::_manual_s::operator ()()
{
	if (is_status(MANUAL_NEED_VALVE_UP) && is_status(MANUAL_NEED_VALVE_DOWN)) {
		fsm.push_event(plate_stop_e{});
	} else if (is_status(MANUAL_NEED_VALVE_UP)) {
		fsm.push_event(plate_up_e{});
	} else if (is_status(MANUAL_NEED_VALVE_DOWN)) {
		fsm.push_event(plate_down_e{});
	}

	if (has_errors()) {
		fsm.push_event(error_e{});
	}
}

void App::_auto_s::operator ()()
{
	static SENSOR_MODE lastMode = SENSOR_MODE_SURFACE;
	if (lastMode != sensorMode) {
		lastMode = sensorMode;
		fsm.push_event(auto_e{});
	}

	if (!workTimer.wait()) {
		stop();
	}

	if (!sampleTimer.wait()) {
		fsm.push_event(timeout_e{});
	}

	if (has_errors()) {
		fsm.push_event(error_e{});
	}
}

void App::_up_s::operator ()()
{
	if ((is_status(MANUAL_NEED_VALVE_UP) && is_status(MANUAL_NEED_VALVE_DOWN)) ||
		!is_status(MANUAL_NEED_VALVE_UP) ||
		has_errors()
	) {
		fsm.push_event(plate_stop_e{});
	}
}

void App::_down_s::operator ()()
{
	if ((is_status(MANUAL_NEED_VALVE_UP) && is_status(MANUAL_NEED_VALVE_DOWN)) ||
		!is_status(MANUAL_NEED_VALVE_DOWN) ||
		has_errors()
	) {
		fsm.push_event(plate_stop_e{});
	}
}

void App::_error_s::operator ()()
{
	if (!has_errors()) {
		fsm.push_event(solved_e{});
		reset_error(VALVE_ERROR);
	}
}

void App::manual_start_a::operator ()()
{
	stop();
}

void App::auto_start_a::operator ()()
{
	if (sensorMode == get_sensor_mode()) {
		return;
	}

	switch(get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		deadBand = DEAD_BANDS_MMx10[settings.surface_snstv];
		propBand = PROP_BANDS_MMx10[settings.surface_snstv];
		break;
	case SENSOR_MODE_STRING:
		deadBand = DEAD_BANDS_MMx10[settings.string_snstv];
		propBand = PROP_BANDS_MMx10[settings.string_snstv];
		break;
	case SENSOR_MODE_BIGSKI:
		deadBand = DEAD_BANDS_MMx10[settings.bigski_snstv];
		propBand = PROP_BANDS_MMx10[settings.bigski_snstv];
		break;
	default:
		BEDUG_ASSERT(false, "Unknown mode");
		fsm.push_event(error_e{});
		Error_Handler();
		break;
	}
}

void App::setup_move_a::operator ()()
{
	if (get_sensor_mode() == SENSOR_MODE_BIGSKI && is_status(NO_BIGSKI)) {
		stop();
		return;
	}

	if (__abs(getValue()) < deadBand) {
		stop();
		return;
	}

	if (__abs(getValue()) > propBand) {
		getValue() > 0 ? down() : up();
		return;
	}

	if (sampleTimer.wait()) {
		return;
	}

	if (workTimer.wait()) {
		return;
	}

	uint32_t propPercent = (__abs_dif(propBand, getValue()) * 100) / propBand;
	uint32_t timeMs = WORK_COEFFICIENT * propPercent / 100;

	workTimer.changeDelay(timeMs);

	getValue() > 0 ? down() : up();

	sampleTimer.start();
	workTimer.start();
}

void App::move_up_a::operator ()()
{
	up();
}

void App::move_down_a::operator ()()
{
	down();
}

void App::plate_stop_a::operator ()()
{
	stop();
}

void App::error_start_a::operator ()()
{
	stop();
}
