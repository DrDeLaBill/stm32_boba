/* Copyright © 2024 Georgy E. All rights reserved. */

#include "App.h"

#include "glog.h"
#include "main.h"
#include "soul.h"
#include "sensor.h"
#include "settings.h"
#include "hal_defs.h"


fsm::FiniteStateMachine<App::fsm_table> App::fsm;
uint16_t App::deadBand = 0;
uint16_t App::propBand = 0;
utl::Timer App::sampleTimer(App::SAMPLE_PWM_MS);
utl::Timer App::sensDelayTimer(0);
utl::Timer App::workTimer(0);
SENSOR_MODE App::sensorMode = SENSOR_MODE_SURFACE;
APP_MODE App::appMode = APP_MODE_MANUAL;
App::SENSOR_POSITION App::position = App::ON_INIT;
App::buffer_t App::value_buffer;



App::App(): measureTimer(0) {}

void App::proccess()
{
	fsm.proccess();

	if (measureTimer.wait()) {
		return;
	}
	measureTimer.start();

	value_buffer.pop_back();
	value_buffer.push_front(getCurrentSensorValue());
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

int16_t App::getRealValue()
{
	return value_buffer.front();
}

int16_t App::getActualValue()
{
	return value_buffer.back();
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

uint16_t App::getDeadBand()
{
	switch(get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		return DEAD_BANDS_MMx10[settings.surface_snstv];
	case SENSOR_MODE_STRING:
		return DEAD_BANDS_MMx10[settings.string_snstv];
	case SENSOR_MODE_BIGSKI:
		return DEAD_BANDS_MMx10[settings.bigski_snstv];
	default:
		BEDUG_ASSERT(false, "Unknown mode");
		fsm.push_event(error_e{});
		Error_Handler();
		return 0;
	}
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

bool App::isOnDeadBand()
{
	return __abs(getActualValue()) <= deadBand;
}

bool App::isOnPropBand()
{
	return __abs(getActualValue()) > deadBand && __abs(getActualValue()) <= propBand;
}

int16_t App::getCurrentSensorValue()
{
	if (get_sensor_mode() == SENSOR_MODE_BIGSKI) {
		return get_sensor_average();
	}

	return get_sensor2A7_value();
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
	if (has_errors()) {
		fsm.push_event(error_e{});
	}

	if (getActualValue() == SENSOR_VALUE_ERR) {
		stop();
		return;
	}

	static SENSOR_MODE lastMode = SENSOR_MODE_SURFACE;
	if (lastMode != sensorMode) {
		lastMode = sensorMode;
		fsm.push_event(auto_e{});
	}

	if (!sensor2A7_available()) {
		setAppMode(APP_MODE_MANUAL);
		stop();
		return;
	}

	if (isOnDeadBand()) {
		position = ON_DEAD_BAND;
		stop();
		return;
	}

	if (!isOnPropBand()) {
		position = ON_PROP_BAND;
		getActualValue() > 0 ? down() : up();
		return;
	}

	if (position != ON_PROP_BAND) {
		position = ON_PROP_BAND;
		sensDelayTimer.start();
		sampleTimer.reset();
		workTimer.reset();
	}

	if (sensDelayTimer.wait()) {
		return;
	}

	if (workTimer.wait()) {
		return;
	} else {
		stop();
	}

	if (sampleTimer.wait()) {
		return;
	}

	if (!propBand) {
		BEDUG_ASSERT(false, "Prop band error");
		fsm.push_event(error_e{});
		return;
	}

	uint32_t k_percent = (__abs_dif(propBand, __abs(getActualValue())) * 100) / propBand;
	uint32_t time_ms = (k_percent * SAMPLE_PWM_MS) / 100;

	if (!time_ms) {
		return;
	}

	if (time_ms < VALVE_MIN_TIME_MS) {
		time_ms = VALVE_MIN_TIME_MS;
	}

	workTimer.changeDelay(time_ms);

	getActualValue() > 0 ? down() : up();

	sampleTimer.start();
	workTimer.start();
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

	int16_t lastValue = value_buffer.front();
	value_buffer.clear();
	value_buffer.push_front(lastValue);
}

void App::auto_start_a::operator ()()
{
	uint32_t measureCount = 0;
	switch(get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		deadBand = DEAD_BANDS_MMx10[settings.surface_snstv];
		propBand = PROP_BANDS_MMx10[settings.surface_snstv];
		sensDelayTimer.changeDelay(SENSITIVITY_DELAY_MS[settings.surface_snstv]);
		measureCount = settings.surface_delay * WORK_DELAY_BUFFER_MS;
		break;
	case SENSOR_MODE_STRING:
		deadBand = DEAD_BANDS_MMx10[settings.string_snstv];
		propBand = PROP_BANDS_MMx10[settings.string_snstv];
		sensDelayTimer.changeDelay(SENSITIVITY_DELAY_MS[settings.string_snstv]);
		measureCount = settings.string_delay * WORK_DELAY_BUFFER_MS;
		break;
	case SENSOR_MODE_BIGSKI:
		deadBand = DEAD_BANDS_MMx10[settings.bigski_snstv];
		propBand = PROP_BANDS_MMx10[settings.bigski_snstv];
		sensDelayTimer.changeDelay(SENSITIVITY_DELAY_MS[settings.bigski_snstv]);
		measureCount = settings.bigski_delay * WORK_DELAY_BUFFER_MS;
		break;
	default:
		BEDUG_ASSERT(false, "Unknown mode");
		fsm.push_event(error_e{});
		Error_Handler();
		return;
	}

	if (!measureCount) {
		measureCount = 1;
	}
	int16_t lastValue = value_buffer.front();
	value_buffer.clear();
	for (unsigned i = 0; i < measureCount; i++) {
		value_buffer.push_front(lastValue);
	}
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
