/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Button.h"


Button::Button(GPIO_TypeDef* port, uint16_t pin, bool inverse):
	debounce(DEBOUNCE_MS), port(port), pin(pin), currState(inverse), inverse(inverse), clicked(false)
{}

void Button::tick()
{
	if (debounce.wait()) {
		return;
	}
	debounce.start();

	bool tmpState = HAL_GPIO_ReadPin(const_cast<GPIO_TypeDef*>(port), pin);
	currState = inverse ? !tmpState : tmpState;

	if (currState) {
		clicked = true;
	}
}

bool Button::wasClicked()
{
	if (clicked) {
		clicked = false;
		return true;
	}
	return false;
}
