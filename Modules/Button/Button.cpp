/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Button.h"

#include "gtime.h"
#include "gutils.h"


Button::Button():
	debounce(BUTTON_DEBOUNCE_MS), port(0),
	pin(0), currState(0), inverse(0), clicked(false),
	HOLD_TIME_MS(Button::DEFAULT_HOLD_TIME_MS), holdStart(0)
{}

Button::Button(GPIO_TypeDef* port, uint16_t pin, bool inverse, uint32_t holdTime):
	debounce(BUTTON_DEBOUNCE_MS), port(port),
	pin(pin), currState(!inverse), inverse(inverse), clicked(false),
	HOLD_TIME_MS(holdTime), holdStart(0)
{}

void Button::tick()
{
	if (debounce.wait()) {
		return;
	}

	bool state = pressed();
	if (!state) {
		holdStart = 0;
	}
	if (state == currState) {
		return;
	}

	debounce.start();

	if (state) {
		clicked = false;
		holdStart = getMillis();
	} else {
		clicked = true;
	}
	currState = state;
}

bool Button::oneClick()
{
	if (clicked) {
		clicked = false;
		return true;
	}
	return false;
}

bool Button::isHolded()
{
	if (!pressed()) {
		return false;
	}
	return __abs_dif(getMillis(), holdStart) > HOLD_TIME_MS;
}

bool Button::pressed()
{
	bool state = HAL_GPIO_ReadPin(const_cast<GPIO_TypeDef*>(port), pin);
	return inverse ? !state : state;
}
