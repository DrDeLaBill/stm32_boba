/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _BUTTON_H_
#define _BUTTON_H_


#include <stdint.h>

#include "Timer.h"
#include "hal_defs.h"


#define BUTTON_DEBOUNCE_MS (60)


class Button
{
private:
	static constexpr uint32_t DEFAULT_HOLD_TIME_MS = 1500;

	utl::Timer debounce;
	GPIO_TypeDef* port;
	uint16_t pin;
	bool currState;
	bool inverse;
	bool clicked;

	uint32_t HOLD_TIME_MS;
	uint32_t holdStart;

public:
	Button();
	Button(GPIO_TypeDef* port, uint16_t pin, bool inverse = false, uint32_t holdTime = DEFAULT_HOLD_TIME_MS);

	void tick();

	bool oneClick();
	bool isHolded();
	bool pressed();

};


#endif
