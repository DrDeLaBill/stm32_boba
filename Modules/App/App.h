/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _APP_H_
#define _APP_H_


#include "UI.h"
#include "Timer.h"
#include "FiniteStateMachine.h"


typedef enum _APP_MODE {
	APP_MODE_MANUAL = 1,
	APP_MODE_AUTO
} APP_MODE;


struct App
{
protected:
	static constexpr char TAG[] = "APP";

	static constexpr uint32_t SAMPLE_PWM_MS = 1100;
	static constexpr uint32_t VALVE_MIN_TIME_MS = 100;

	// Events:
	FSM_CREATE_EVENT(success_e,     0);
	FSM_CREATE_EVENT(timeout_e,     0);
	FSM_CREATE_EVENT(plate_up_e,    0);
	FSM_CREATE_EVENT(plate_down_e,  0);
	FSM_CREATE_EVENT(solved_e,      0);
	FSM_CREATE_EVENT(manual_e,      1);
	FSM_CREATE_EVENT(auto_e,        1);
	FSM_CREATE_EVENT(plate_stop_e,  2);
	FSM_CREATE_EVENT(error_e,       3);

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
	struct auto_start_a    { void operator()(); };
	struct setup_move_a    { void operator()(); };
	struct move_up_a       { void operator()(); };
	struct move_down_a     { void operator()(); };
	struct plate_stop_a    { void operator()(); };
	struct error_start_a   { void operator()(); };


	using fsm_table = fsm::TransitionTable<
		fsm::Transition<init_s,   success_e,     manual_s, manual_start_a>,

		fsm::Transition<manual_s, auto_e,        auto_s,   auto_start_a>,
		fsm::Transition<manual_s, plate_up_e,    up_s,     move_up_a>,
		fsm::Transition<manual_s, plate_down_e,  down_s,   move_down_a>,
		fsm::Transition<manual_s, plate_stop_e,  manual_s, plate_stop_a>,
		fsm::Transition<manual_s, error_e,       error_s,  error_start_a>,

		fsm::Transition<auto_s,   auto_e,        auto_s,   auto_start_a>,
		fsm::Transition<auto_s,   manual_e,      manual_s, manual_start_a>,
		fsm::Transition<auto_s,   timeout_e,     auto_s,   setup_move_a>,
		fsm::Transition<auto_s,   error_e,       error_s,  error_start_a>,

		fsm::Transition<up_s,     plate_stop_e,  manual_s, plate_stop_a>,

		fsm::Transition<down_s,   plate_stop_e,  manual_s, plate_stop_a>,

		fsm::Transition<error_s,  solved_e,      manual_s, manual_start_a>
	>;

	static fsm::FiniteStateMachine<fsm_table> fsm;
	static uint16_t deadBand;
	static uint16_t propBand;
	static utl::Timer sampleTimer;
	static utl::Timer workTimer;

	static SENSOR_MODE sensorMode;
	static APP_MODE appMode;
	static int16_t value;

	static void up();
	static void down();
	static void stop();

	static void updatePID();


public:
	void proccess();

	static int16_t getValue();

	static void setAppMode(APP_MODE mode);
	static APP_MODE getAppMode();

	static void changeSensorMode(SENSOR_MODE mode);

	static uint16_t getDeadBand();

};


#endif
