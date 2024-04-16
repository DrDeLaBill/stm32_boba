/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _BUTTON_H_
#define _BUTTON_H_


#include <stdint.h>

#include "Timer.h"
#include "hal_defs.h"


class Button
{
private:
	static constexpr uint32_t DEBOUNCE_MS = 20;

	utl::Timer debounce;
	const GPIO_TypeDef* port;
	const uint16_t pin;
	bool currState;
	bool inverse;
	bool clicked;

public:
	Button(GPIO_TypeDef* port, uint16_t pin, bool inverse = false);

	void tick();

	bool wasClicked();

};


#endif
