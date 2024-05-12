/* Copyright © 2024 Georgy E. All rights reserved. */

#include "App.h"

#include "log.h"
#include "main.h"
#include "soul.h"
#include "sensor.h"
#include "settings.h"
#include "hal_defs.h"


#define APP_PID_MIN   (-1000)
#define APP_PID_MAX   (1000)

#define VALVE_UP_START()    HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_SET);
#define VALVE_DOWN_START()  HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_SET);
#define VALVE_UP_STOP()     HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_RESET);
#define VALVE_DOWN_STOP()   HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_RESET);
#define VALVE_STOP()        VALVE_UP_STOP(); VALVE_DOWN_STOP();


fsm::FiniteStateMachine<App::fsm_table> App::fsm;
utl::Timer App::samplingTimer(0);
utl::Timer App::valveTimer(0);
GyverPID* App::pid;;
SENSOR_MODE App::sensorMode = SENSOR_MODE_SURFACE;
APP_MODE App::appMode = APP_MODE_MANUAL;


void App::proccess()
{
	fsm.proccess();
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

APP_MODE App::getAppMode()
{
	return appMode;
}

void App::changeSensorMode(SENSOR_MODE mode)
{
	set_sensor_mode(mode);
	sensorMode = mode;
}

void App::stop()
{
	fsm.clear_events();
	VALVE_STOP();
}

void App::_init_s::operator ()()
{
	VALVE_STOP();

	if (is_status(WAIT_LOAD)) {
		return;
	}

	pid = new GyverPID(
		settings.surface_pid.kp,
		settings.surface_pid.ki,
		settings.surface_pid.kd,
		settings.surface_pid.sampling
	);
	pid->setDirection(NORMAL);
	pid->setLimits(APP_PID_MIN, APP_PID_MAX);

	samplingTimer.changeDelay(settings.surface_pid.sampling);
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
	if (!samplingTimer.wait()) {
		fsm.push_event(pid_timeout_e{});
	}

	if (!valveTimer.wait() && __abs_dif(samplingTimer.end(), getMillis()) > VALVE_MIN_TIME_MS) {
		reset_status(AUTO_NEED_VALVE_DOWN);
		reset_status(AUTO_NEED_VALVE_UP);
		VALVE_STOP();
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
	}
}

void App::manual_start_a::operator ()()
{
	VALVE_STOP();
}

void App::auto_start_a::operator ()()
{
	if (sensorMode == get_sensor_mode()) {
		return;
	}

	pid->reset();

	switch(get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		pid->Kp = settings.surface_pid.kp;
		pid->Ki = settings.surface_pid.ki;
		pid->Kd = settings.surface_pid.kd;
		pid->setDt(settings.surface_pid.sampling);

		samplingTimer.changeDelay(settings.surface_pid.sampling);
		break;
	case SENSOR_MODE_STRING:
		pid->Kp = settings.string_pid.kp;
		pid->Ki = settings.string_pid.ki;
		pid->Kd = settings.string_pid.kd;
		pid->setDt(settings.string_pid.sampling);

		samplingTimer.changeDelay(settings.string_pid.sampling);
		break;
	case SENSOR_MODE_BIGSKY:
		pid->Kp = settings.bigsky_pid.kp;
		pid->Ki = settings.bigsky_pid.ki;
		pid->Kd = settings.bigsky_pid.kd;
		pid->setDt(settings.bigsky_pid.sampling);

		samplingTimer.changeDelay(settings.bigsky_pid.sampling);
		break;
	default:
		BEDUG_ASSERT(false, "Unknown button in buffer");
		fsm.push_event(error_e{});
		Error_Handler();
		break;
	}

	samplingTimer.reset();
}

void App::setup_pid_a::operator ()()
{
	int16_t newValue = get_sensor_average_value();

	if (__abs_dif(newValue, settings.last_target) < TRIG_VALUE_LOW) {
		pid->input = settings.last_target;
	} else {
		pid->input = newValue;
	}
	pid->setpoint = 0;

	int16_t pid_ms = pid->getResult();
	if (__abs(pid_ms) < VALVE_MIN_TIME_MS) {
		valveTimer.changeDelay(0);
		reset_status(AUTO_NEED_VALVE_DOWN);
		reset_status(AUTO_NEED_VALVE_UP);
		VALVE_STOP();
	} else if (pid_ms < 0) {
		valveTimer.changeDelay(static_cast<uint32_t>(__abs(pid_ms)));
		reset_status(AUTO_NEED_VALVE_UP);
		set_status(AUTO_NEED_VALVE_DOWN);
		VALVE_UP_STOP();
		VALVE_DOWN_START();
	} else {
		valveTimer.changeDelay(static_cast<uint32_t>(pid_ms));
		set_status(AUTO_NEED_VALVE_UP);
		reset_status(AUTO_NEED_VALVE_DOWN);
		VALVE_DOWN_STOP();
		VALVE_UP_START();
	}

	samplingTimer.start();
	valveTimer.start();

	if (pid_ms) {
		printTagLog(TAG, "value: %06d(%06d), new PID: %d ms", pid->input, newValue, pid_ms);
	}
}

void App::move_up_a::operator ()()
{
	VALVE_DOWN_STOP();
	VALVE_UP_START();
}

void App::move_down_a::operator ()()
{
	VALVE_UP_STOP();
	VALVE_DOWN_START();
}

void App::plate_stop_a::operator ()()
{
	VALVE_STOP();
}

void App::error_start_a::operator ()()
{
	VALVE_STOP();
}
