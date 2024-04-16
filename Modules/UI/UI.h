/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _UI_H_
#define _UI_H_


#include <cstdint>

#include "Timer.h"
#include "Button.h"
#include "CircleBuffer.h"
#include "FiniteStateMachine.h"


struct UI
{
private:
	static constexpr unsigned UI_CLICKS_SIZE = 8;

	static constexpr uint32_t DEBOUNCE_MS = 20;

	static constexpr unsigned BUTTONS_COUNT = 4;

protected:
	static constexpr uint16_t DEFAULT_MARGIN = 10;

	// Events:
	FSM_CREATE_EVENT(success_e,     0);
	FSM_CREATE_EVENT(no_sens_e,     0);
	FSM_CREATE_EVENT(sens_found_e,  0);
	FSM_CREATE_EVENT(change_mode_e, 0);
	FSM_CREATE_EVENT(error_e,       1);


	// States:
	struct _init_s        { void operator()() const; };
	struct _load_s        { void operator()() const; };
	struct _no_sens_s     { void operator()() const; };
	struct _manual_mode_s { void operator()() const; };
	struct _auto_mode_s   { void operator()() const; };
	struct _error_s       { void operator()() const; };

	FSM_CREATE_STATE(init_s,        _init_s);
	FSM_CREATE_STATE(load_s,        _load_s);
	FSM_CREATE_STATE(no_sens_s,     _no_sens_s);
	FSM_CREATE_STATE(manual_mode_s, _manual_mode_s);
	FSM_CREATE_STATE(auto_mode_s,   _auto_mode_s);
	FSM_CREATE_STATE(error_s,       _error_s);


	// Actions:
	struct none_a          { void operator()() const; };
	struct load_start_a    { void operator()() const; };
	struct no_sens_start_a { void operator()() const; };
	struct manual_start_a  { void operator()() const; };
	struct auto_start_a    { void operator()() const; };


	// FSM:
	using fsm_table = fsm::TransitionTable<
		fsm::Transition<init_s,        success_e,     load_s,        load_start_a>,

		fsm::Transition<load_s,        success_e,     manual_mode_s, manual_start_a>,
		fsm::Transition<load_s,        no_sens_e,     no_sens_s,     no_sens_start_a>,
		fsm::Transition<load_s,        error_e,       error_s,       none_a>,

		fsm::Transition<no_sens_s,     sens_found_e,  manual_mode_s, manual_start_a>,

		fsm::Transition<manual_mode_s, change_mode_e, auto_mode_s,   auto_start_a>,
		fsm::Transition<manual_mode_s, no_sens_e,     no_sens_s,     no_sens_start_a>,
		fsm::Transition<manual_mode_s, error_e,       error_s,       none_a>,

		fsm::Transition<auto_mode_s,   change_mode_e, manual_mode_s, manual_start_a>,
		fsm::Transition<auto_mode_s,   no_sens_e,     no_sens_s,     no_sens_start_a>,
		fsm::Transition<auto_mode_s,   error_e,       error_s,       none_a>
	>;
	static fsm::FiniteStateMachine<fsm_table> fsm;

	static utl::Timer timer;


	static void showHeader();
	static void showFooter();

	static void showValue();


public:
	static constexpr char TAG[] = "UI";

	typedef enum _button_t {
		BUTTON_UP = 0,
		BUTTON_DOWN,
		BUTTON_MODE,
		BUTTON_ENTER,
		BUTTON_F1,
		BUTTON_F2,
		BUTTON_F3
	} button_t;

	static utl::circle_buffer<UI_CLICKS_SIZE, button_t> clicks;
	static Button buttons[];

	static void showUp(bool flag = true);
	static void showDown(bool flag = true);

	void tick();

};


#endif
