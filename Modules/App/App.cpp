/* Copyright © 2024 Georgy E. All rights reserved. */

#include "App.h"

#include "main.h"
#include "soul.h"
#include "hal_defs.h"


#define VALVE_UP()   HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_SET);
#define VALVE_DOWN() HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_SET);
#define VALVE_STOP() HAL_GPIO_WritePin(VALVE_UP_GPIO_Port, VALVE_UP_Pin, GPIO_PIN_RESET); HAL_GPIO_WritePin(VALVE_DOWN_GPIO_Port, VALVE_DOWN_Pin, GPIO_PIN_RESET);


fsm::FiniteStateMachine<App::fsm_table> App::fsm;
utl::Timer App::timer(0);
GyverPID App::pid(1.0f, 0.3f, 0.1f, 500); // TODO
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
		set_error(INTERNAL_ERROR);
		break;
	};
}

void App::stop()
{
	fsm.clear_events();
	VALVE_STOP();
}

void App::_init_s::operator ()()
{
	pid.setDirection(REVERSE);
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

}

void App::ground_start_a::operator ()()
{

}

void App::string_start_a::operator ()()
{

}

void App::setup_pid_a::operator ()()
{

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
