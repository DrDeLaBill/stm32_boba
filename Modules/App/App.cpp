/* Copyright © 2024 Georgy E. All rights reserved. */

#include "App.h"

#include "glog.h"
#include "main.h"
#include "sensor.h"
#include "gsystem.h"
#include "settings.h"
#include "hal_defs.h"


static App app;

fsm::FiniteStateMachine<App::fsm_table> App::fsm;
uint16_t App::deadBand = 0;
uint16_t App::propBand = 0;
utl::Timer App::sampleTimer(App::SAMPLE_PWM_MS);
utl::Timer App::sensDelayTimer(1);
utl::Timer App::workTimer(1);
utl::Timer App::noiseTimer(800);
SENSOR_MODE App::sensorMode = SENSOR_MODE_SURFACE;
APP_MODE App::appMode = APP_MODE_MANUAL;
App::SENSOR_POSITION App::position = App::ON_INIT;
App::buffer_t App::value_buffer;


void app_tick()
{
	app.process();
}

App::App(): measureTimer(MEAS_DELAY_MS) {}

void App::process()
{
	fsm.proccess();

	if (measureTimer.wait()) {
		return;
	}
	measureTimer.start();

	if (!value_buffer.empty()) {
		value_buffer.pop_back();
	}

	int16_t value = get_sensor_value();
	if (get_sensor_mode() == SENSOR_MODE_ANGLE) {
		value -= settings.angle_target;
	}
	value_buffer.push_front(value);
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
	uint8_t sensitive = get_sensor_mode_sensitive();
	switch(get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		return DEAD_BANDS_MMx10[sensitive];
	case SENSOR_MODE_STRING:
		return DEAD_BANDS_MMx10[sensitive];
	case SENSOR_MODE_BIGSKI:
		return DEAD_BANDS_MMx10[sensitive];
	case SENSOR_MODE_ANGLE:
		return ANGLE_DEAD_BANDS[sensitive];
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

uint16_t App::getAppDeadBand()
{
	if (position == ON_DEAD_BAND) {
		return deadBand;
	}
	return deadBand / 2;
}

bool App::isOnDeadBand()
{
	return __abs(getActualValue()) <= getAppDeadBand();
}

bool App::isOnPropBand()
{
	return __abs(getActualValue()) <= propBand;
}

void App::_init_s::operator ()()
{
	stop();

	if (!is_system_ready()) {
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

	if (!sensor_available()) {
		if (is_status(MANUAL_NEED_VALVE_UP) && is_status(MANUAL_NEED_VALVE_DOWN)) {
			stop();
		} else if (is_status(MANUAL_NEED_VALVE_UP)) {
			up();
		} else if (is_status(MANUAL_NEED_VALVE_DOWN)) {
			down();
		} else {
			stop();
		}
		return;
	}

	if (isOnDeadBand()) {
		position = ON_DEAD_BAND;
		noiseTimer.start();
		stop();
		return;
	}

	if (noiseTimer.wait()) {
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

	const uint32_t MAX_PERCENTS[] = {
		80,
		80,
		70,
		60,
		40,
		40,
		30,
		20,
		20,
		10
	};
	uint32_t max_percent = MAX_PERCENTS[get_sensor_mode_sensitive()];
	uint32_t k_percent = max_percent - (
		(
			__abs_dif(
				__abs(propBand),
				__abs(getActualValue())
			) * max_percent
		) / propBand
	);
	uint32_t time_ms = (k_percent * SAMPLE_PWM_MS) / 100;

	if (time_ms < VALVE_MIN_TIME_MS) {
		stop();
		return;
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

	int16_t lastValue = 0;
	if (!value_buffer.empty()) {
	    lastValue = value_buffer.front();
	}
	value_buffer.clear();
	value_buffer.push_front(lastValue);
}

void App::auto_start_a::operator ()()
{
	uint32_t measureCount = 0;
	uint8_t sensitive = get_sensor_mode_sensitive();
	switch(get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		deadBand = DEAD_BANDS_MMx10[sensitive];
		propBand = PROP_BANDS_MMx10[sensitive];
		sensDelayTimer.changeDelay(SENSITIVITY_DELAY_MS[sensitive]);
		measureCount = settings.surface_delay * WORK_DELAY_BUFFER_MS;
		break;
	case SENSOR_MODE_STRING:
		deadBand = DEAD_BANDS_MMx10[sensitive];
		propBand = PROP_BANDS_MMx10[sensitive];
		sensDelayTimer.changeDelay(SENSITIVITY_DELAY_MS[sensitive]);
		measureCount = settings.string_delay * WORK_DELAY_BUFFER_MS;
		break;
	case SENSOR_MODE_BIGSKI:
		deadBand = DEAD_BANDS_MMx10[sensitive];
		propBand = PROP_BANDS_MMx10[sensitive];
		sensDelayTimer.changeDelay(SENSITIVITY_DELAY_MS[sensitive]);
		measureCount = settings.bigski_delay * WORK_DELAY_BUFFER_MS;
		break;
	case SENSOR_MODE_ANGLE:
		deadBand = ANGLE_DEAD_BANDS[sensitive];
		propBand = ANGLE_PROP_BANDS[sensitive];
		sensDelayTimer.changeDelay(SENSITIVITY_DELAY_MS[sensitive]);
		measureCount = settings.angle_delay * WORK_DELAY_BUFFER_MS;
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
