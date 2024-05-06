/* Copyright © 2024 Georgy E. All rights reserved. */

#include "App.h"

#include "log.h"
#include "main.h"
#include "soul.h"
#include "sensor.h"
#include "settings.h"
#include "hal_defs.h"


#define APP_PID_MIN   (-7000)
#define APP_PID_MAX   (7000)

#define VALVE_UP()    HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_SET);
#define VALVE_DOWN()  HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_SET);
#define VALVE_STOP()  HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_RESET); HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_RESET);


fsm::FiniteStateMachine<App::fsm_table> App::fsm;
utl::Timer App::samplingTimer(0);
utl::Timer App::valveTimer(0);
GyverPID* App::pid;;
APP_mode_t App::mode = APP_MODE_MANUAL;
UI App::ui;


void App::proccess()
{
	ui.tick();
	fsm.proccess();
}

void App::setMode(APP_mode_t mode)
{
	switch(mode) {
	case APP_MODE_MANUAL:
		fsm.push_event(manual_e{});
		break;
	case APP_MODE_SURFACE:
		fsm.push_event(surface_e{});
		break;
	case APP_MODE_GROUND:
		fsm.push_event(ground_e{});
		break;
	case APP_MODE_STRING:
		fsm.push_event(string_e{});
		break;
	default:
		fsm.push_event(error_e{});
		set_error(APP_MODE_ERROR);
		return;
	};
	App::mode = mode;
}

APP_mode_t App::getMode()
{
	return mode;
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

void App::surface_start_a::operator ()()
{
	pid->Kp = settings.surface_pid.kp;
	pid->Ki = settings.surface_pid.ki;
	pid->Kd = settings.surface_pid.kd;
	pid->setDt(settings.surface_pid.sampling);

	samplingTimer.changeDelay(settings.surface_pid.sampling);
	samplingTimer.reset();
}

void App::ground_start_a::operator ()()
{
	pid->Kp = settings.ground_pid.kp;
	pid->Ki = settings.ground_pid.ki;
	pid->Kd = settings.ground_pid.kd;
	pid->setDt(settings.ground_pid.sampling);

	samplingTimer.changeDelay(settings.ground_pid.sampling);
	samplingTimer.reset();
}

void App::string_start_a::operator ()()
{
	pid->Kp = settings.string_pid.kp;
	pid->Ki = settings.string_pid.ki;
	pid->Kd = settings.string_pid.kd;
	pid->setDt(settings.string_pid.sampling);

	samplingTimer.changeDelay(settings.string_pid.sampling);
	samplingTimer.reset();
}

void App::setup_pid_a::operator ()()
{
	int16_t newValue = get_sensor_value();

	if (__abs_dif(newValue, settings.last_target) < TRIG_VALUE_LOW) {
		pid->input = settings.last_target;
	} else {
		pid->input = newValue;
	}
	pid->setpoint = settings.last_target;

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
		VALVE_DOWN();
	} else {
		valveTimer.changeDelay(static_cast<uint32_t>(pid_ms));
		set_status(AUTO_NEED_VALVE_UP);
		reset_status(AUTO_NEED_VALVE_DOWN);
		VALVE_UP();
	}

	samplingTimer.start();
	valveTimer.start();

	if (pid_ms) {
		printTagLog(TAG, "value: %06d(%06d), new PID: %d ms", pid->input, newValue, pid_ms);
	}
}

void App::move_up_a::operator ()()
{
	VALVE_UP();
}

void App::move_down_a::operator ()()
{
	VALVE_DOWN();
}

void App::plate_stop_a::operator ()()
{
	VALVE_STOP();
}

void App::error_start_a::operator ()()
{

}
