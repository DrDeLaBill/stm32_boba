/* Copyright © 2024 Georgy E. All rights reserved. */

#include "App.h"

#include "log.h"
#include "main.h"
#include "soul.h"
#include "sensor.h"
#include "settings.h"
#include "hal_defs.h"


fsm::FiniteStateMachine<App::fsm_table> App::fsm;
utl::Timer App::samplingTimer(0);
utl::Timer App::valveTimer(0);
GyverPID* App::pid;;
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

	pid = new GyverPID(
		settings.surface.kp,
		settings.surface.ki,
		settings.surface.kd,
		settings.surface.sampling
	);
	pid->setpoint = 0;
	pid->setDirection(NORMAL);
	pid->setLimits(-settings.max_pid_time, settings.max_pid_time);

	samplingTimer.changeDelay(settings.surface.sampling);
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

	if (!samplingTimer.wait()) {
		fsm.push_event(pid_timeout_e{});
	}

	if (!valveTimer.wait() && __abs_dif(samplingTimer.end(), getMillis()) > VALVE_MIN_TIME_MS) {
		reset_status(AUTO_NEED_VALVE_DOWN);
		reset_status(AUTO_NEED_VALVE_UP);
		stop();
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

	pid->reset();

	switch(get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		pid->Kp = settings.surface.kp;
		pid->Ki = settings.surface.ki;
		pid->Kd = settings.surface.kd;
		pid->setDt(settings.surface.sampling);

		samplingTimer.changeDelay(settings.surface.sampling);
		break;
	case SENSOR_MODE_STRING:
		pid->Kp = settings.string.kp;
		pid->Ki = settings.string.ki;
		pid->Kd = settings.string.kd;
		pid->setDt(settings.string.sampling);

		samplingTimer.changeDelay(settings.string.sampling);
		break;
	case SENSOR_MODE_BIGSKI:
		pid->Kp = settings.bigski.kp;
		pid->Ki = settings.bigski.ki;
		pid->Kd = settings.bigski.kd;
		pid->setDt(settings.bigski.sampling);

		samplingTimer.changeDelay(settings.bigski.sampling);
		break;
	default:
		BEDUG_ASSERT(false, "Unknown mode");
		fsm.push_event(error_e{});
		Error_Handler();
		break;
	}

	samplingTimer.reset();
}

void App::setup_pid_a::operator ()()
{
	pid->setLimits(-settings.max_pid_time, settings.max_pid_time);

	if (get_sensor_mode() == SENSOR_MODE_BIGSKI && is_status(NO_BIGSKI)) {
		pid->reset();
		stop();
		return;
	}

	if (__abs_dif(value, 0) < TRIG_VALUE_LOW) {
		pid->input = 0;
	} else {
		pid->input = value;
	}

	int16_t pid_ms = pid->getResult();
	if (__abs(pid_ms) < VALVE_MIN_TIME_MS) {
		stop();
	} else if (pid_ms < 0) {
		valveTimer.changeDelay(static_cast<uint32_t>(__abs(pid_ms)));
		down();
	} else {
		valveTimer.changeDelay(static_cast<uint32_t>(pid_ms));
		up();
	}

	samplingTimer.start();
	valveTimer.start();

	if (pid_ms) {
		printTagLog(TAG, "value: %06d(%06d), new PID: %d ms", pid->input, value, pid_ms);
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
