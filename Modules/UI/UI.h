/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _UI_H_
#define _UI_H_


#include <memory>
#include <utility>
#include <cstdint>
#include <unordered_map>

#include "Menu.h"
#include "Timer.h"
#include "Button.h"
#include "CircleBuffer.h"
#include "FiniteStateMachine.h"


#define UI_BEDUG (false)


struct UI
{
private:
	static constexpr unsigned UI_CLICKS_SIZE = 8;

	static constexpr uint32_t DEBOUNCE_MS = 20;

protected:
	static constexpr uint16_t DEFAULT_MARGIN = 10;

	static constexpr int16_t TRIG_VALUE_LOW = 30;
	static constexpr int16_t TRIG_VALUE_HIGH = 30;

	// Events:
	FSM_CREATE_EVENT(success_e,     0);
	FSM_CREATE_EVENT(sens_found_e,  0);
	FSM_CREATE_EVENT(change_mode_e, 0);
	FSM_CREATE_EVENT(service_e,     1);
	FSM_CREATE_EVENT(no_sens_e,     2);
	FSM_CREATE_EVENT(error_e,       3);


	// States:
	struct _init_s        { void operator()() const; };
	struct _load_s        { void operator()() const; };
	struct _no_sens_s     { void operator()() const; };
	struct _manual_mode_s { void operator()() const; };
	struct _auto_mode_s   { void operator()() const; };
	struct _service_s     { void operator()() const; };
	struct _error_s       { void operator()() const; };

	FSM_CREATE_STATE(init_s,        _init_s);
	FSM_CREATE_STATE(load_s,        _load_s);
	FSM_CREATE_STATE(no_sens_s,     _no_sens_s);
	FSM_CREATE_STATE(manual_mode_s, _manual_mode_s);
	FSM_CREATE_STATE(auto_mode_s,   _auto_mode_s);
	FSM_CREATE_STATE(service_s,     _service_s);
	FSM_CREATE_STATE(error_s,       _error_s);


	// Actions:
	struct error_a         { void operator()() const; };
	struct load_start_a    { void operator()() const; };
	struct no_sens_start_a { void operator()() const; };
	struct manual_start_a  { void operator()() const; };
	struct auto_start_a    { void operator()() const; };
	struct service_start_a { void operator()() const; };


	// FSM:
	using fsm_table = fsm::TransitionTable<
		fsm::Transition<init_s,        success_e,     load_s,        load_start_a>,

		fsm::Transition<load_s,        success_e,     manual_mode_s, manual_start_a>,
		fsm::Transition<load_s,        no_sens_e,     no_sens_s,     no_sens_start_a>,
		fsm::Transition<load_s,        error_e,       error_s,       error_a>,

		fsm::Transition<no_sens_s,     sens_found_e,  manual_mode_s, manual_start_a>,
		fsm::Transition<no_sens_s,     service_e,     service_s,     service_start_a>,
		fsm::Transition<no_sens_s,     error_e,       error_s,       error_a>,

		fsm::Transition<manual_mode_s, change_mode_e, auto_mode_s,   auto_start_a>,
		fsm::Transition<manual_mode_s, no_sens_e,     no_sens_s,     no_sens_start_a>,
		fsm::Transition<manual_mode_s, service_e,     service_s,     service_start_a>,
		fsm::Transition<manual_mode_s, error_e,       error_s,       error_a>,

		fsm::Transition<service_s,     service_e,     manual_mode_s, manual_start_a>,

		fsm::Transition<auto_mode_s,   change_mode_e, manual_mode_s, manual_start_a>,
		fsm::Transition<auto_mode_s,   no_sens_e,     no_sens_s,     no_sens_start_a>,
		fsm::Transition<auto_mode_s,   error_e,       error_s,       error_a>,

		fsm::Transition<error_s,       success_e,     load_s,        load_start_a>
	>;
	static fsm::FiniteStateMachine<fsm_table> fsm;

	static utl::Timer timer;

	static std::unique_ptr<Menu> serviceMenu;

	static void showMode();
	static void showHeader();
	static void showAutoFooter();
	static void showManualFooter();
	static void showServiceFooter();
	static void showValue();
	static void showLoading();


public:
	static constexpr char TAG[] = "UI";

	static utl::circle_buffer<UI_CLICKS_SIZE, uint16_t> clicks;
	static std::unordered_map<uint16_t, Button> buttons;
//	static std::pair<uint16_t, Button> buttons[BUTTONS_COUNT];

	static void showUp(bool flag = false);
	static void showDown(bool flag = false);
	static void showMiddle(bool flag = false);

	void tick();

};


#endif
