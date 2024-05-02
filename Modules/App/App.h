/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _APP_H_
#define _APP_H_


#include "UI.h"
#include "Timer.h"
#include "GyverPID.h"
#include "FiniteStateMachine.h"


typedef enum _APP_mode_t {
	APP_MODE_MANUAL = 1,
	APP_MODE_SURFACE,
	APP_MODE_GROUND,
	APP_MODE_STRING
} APP_mode_t;


struct App
{
protected:
	// Events:
	FSM_CREATE_EVENT(success_e,     0);
	FSM_CREATE_EVENT(manual_e,      0);
	FSM_CREATE_EVENT(surface_e,     0);
	FSM_CREATE_EVENT(ground_e,      0);
	FSM_CREATE_EVENT(string_e,      0);
	FSM_CREATE_EVENT(pid_timeout_e, 0);
	FSM_CREATE_EVENT(plate_up_e,    0);
	FSM_CREATE_EVENT(plate_down_e,  0);
	FSM_CREATE_EVENT(solved_e,      0);
	FSM_CREATE_EVENT(plate_stop_e,  1);
	FSM_CREATE_EVENT(error_e,       2);

	// States:
	struct _init_s   { void operator()(); };
	struct _manual_s { void operator()(); };
	struct _auto_s   { void operator()(); };
	struct _up_s     { void operator()(); };
	struct _down_s   { void operator()(); };
	struct _error_s  { void operator()(); };

	FSM_CREATE_STATE(init_s,   _init_s);
	FSM_CREATE_STATE(manual_s, _manual_s);
	FSM_CREATE_STATE(auto_s,   _auto_s);
	FSM_CREATE_STATE(up_s,     _up_s);
	FSM_CREATE_STATE(down_s,   _down_s);
	FSM_CREATE_STATE(error_s,  _error_s);

	// Actions:
	struct manual_start_a  { void operator()(); };
	struct surface_start_a { void operator()(); };
	struct ground_start_a  { void operator()(); };
	struct string_start_a  { void operator()(); };
	struct setup_pid_a     { void operator()(); };
	struct move_up_a       { void operator()(); };
	struct move_down_a     { void operator()(); };
	struct plate_stop_a    { void operator()(); };
	struct error_start_a   { void operator()(); };


	using fsm_table = fsm::TransitionTable<
		fsm::Transition<init_s,   success_e,    manual_s, manual_start_a>,

		fsm::Transition<manual_s, surface_e,    auto_s,   surface_start_a>,
		fsm::Transition<manual_s, ground_e,     auto_s,   ground_start_a>,
		fsm::Transition<manual_s, string_e,     auto_s,   string_start_a>,
		fsm::Transition<manual_s, plate_up_e,   up_s,     move_up_a>,
		fsm::Transition<manual_s, plate_down_e, down_s,   move_down_a>,
		fsm::Transition<manual_s, plate_stop_e, manual_s, plate_stop_a>,
		fsm::Transition<manual_s, error_e,      error_s,  error_start_a>,

		fsm::Transition<auto_s,  manual_e,      manual_s, manual_start_a>,
		fsm::Transition<auto_s,  surface_e,     auto_s,   surface_start_a>,
		fsm::Transition<auto_s,  ground_e,      auto_s,   ground_start_a>,
		fsm::Transition<auto_s,  string_e,      auto_s,   string_start_a>,
		fsm::Transition<auto_s,  pid_timeout_e, auto_s,   setup_pid_a>,
		fsm::Transition<auto_s,  error_e,       error_s,  error_start_a>,

		fsm::Transition<up_s,    plate_stop_e,  manual_s, plate_stop_a>,

		fsm::Transition<down_s,  plate_stop_e,  manual_s, plate_stop_a>,

		fsm::Transition<error_s, solved_e,      manual_s, manual_start_a>
	>;

	static fsm::FiniteStateMachine<fsm_table> fsm;
	static utl::Timer pidTimer;
	static utl::Timer timer;
	static GyverPID* pid;
	static UI ui;


	static void stop();


public:
	void proccess();

	static void setMode(APP_mode_t mode);

};


#endif
