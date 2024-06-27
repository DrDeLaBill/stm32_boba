/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_


#include <cstdio>
#include <cstring>
#include <cstdint>

#include "main.h"
#include "soul.h"
#include "bmacro.h"

#include "MenuItem.h"


template<class T>
void callback_click(T* target, T step, uint16_t button)
{
	switch (button) {
	case BTN_ENTER_Pin:
		break;
	case BTN_UP_Pin:
		*target += step;
		break;
	case BTN_DOWN_Pin:
		*target -= step;
		break;
	case BTN_MODE_Pin:
	case BTN_F1_Pin:
	case BTN_F2_Pin:
	case BTN_F3_Pin:
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown callback handler");
#endif
		set_error(INTERNAL_ERROR);
		Error_Handler();
		break;
	};
}


struct version_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};


struct language_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};


struct surface_label_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};
struct surface_snstv_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};
struct surface_delay_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};


struct string_label_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};
struct string_snstv_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};
struct string_delay_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};


struct bigski_label_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};
struct bigski_snstv_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};
struct bigski_delay_callback: public IMenuCallback
{
	void click(uint16_t button) override;
	char* value() override;
	char* label() override;
};


#endif
