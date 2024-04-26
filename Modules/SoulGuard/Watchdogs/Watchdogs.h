/* Copyright © 2024 Georgy E. All rights reserved. */

#pragma once


#include <cstdint>

#include "main.h"

#include "hal_defs.h"

#include "Timer.h"
#include "FiniteStateMachine.h"


#define SETTINGS_WATCHDOG_BEDUG (true)
#define POWER_WATCHDOG_BEDUG    (true)


#define WATCHDOG_TIMEOUT_MS     ((uint32_t)100)


/*
 * Filling an empty area of RAM with the STACK_CANARY_WORD value
 * For calculating the RAM fill factor
 */
extern "C" void STACK_WATCHDOG_FILL_RAM(void);


struct StackWatchdog
{
private:
	static constexpr char TAG[] = "STCK";
	static unsigned lastFree;

public:
	void check();

};


struct RestartWatchdog
{
public:
	// TODO: check IWDG or another reboot
	void check();

#ifdef EEPROM_I2C
	static void reset_i2c_errata();
#endif

private:
	static constexpr char TAG[] = "RSTw";
	static bool flagsCleared;

};


struct MemoryWatchdog
{
private:
	static utl::Timer timer;
	static uint8_t errors;

public:
	void check();
};


struct SettingsWatchdog
{
protected:
	struct state_init   {void operator()(void) const;};
	struct state_idle   {void operator()(void) const;};
	struct state_save   {void operator()(void) const;};
	struct state_load   {void operator()(void) const;};

	struct action_check {void operator()(void) const;};
	struct action_reset {void operator()(void) const;};

	FSM_CREATE_STATE(init_s, state_init);
	FSM_CREATE_STATE(idle_s, state_idle);
	FSM_CREATE_STATE(save_s, state_save);
	FSM_CREATE_STATE(load_s, state_load);

	FSM_CREATE_EVENT(event_saved,     0);
	FSM_CREATE_EVENT(event_loaded,    0);
	FSM_CREATE_EVENT(event_updated,   0);
	FSM_CREATE_EVENT(event_not_valid, 1);

	using fsm_table = fsm::TransitionTable<
		fsm::Transition<init_s, event_loaded,    idle_s, action_check>,
		fsm::Transition<init_s, event_saved,     idle_s, action_check>,

		fsm::Transition<idle_s, event_saved,     load_s, action_check>,
		fsm::Transition<idle_s, event_updated,   save_s, action_check>,
		fsm::Transition<idle_s, event_not_valid, save_s, action_reset>,

		fsm::Transition<load_s, event_loaded,    idle_s, action_check>,
		fsm::Transition<save_s, event_saved,     idle_s, action_check>
	>;

	static fsm::FiniteStateMachine<fsm_table> fsm;

private:
	static constexpr char TAG[] = "STGw";

public:
	SettingsWatchdog();

	void check();

};


struct SensorWatchdog
{
	void check();
};
