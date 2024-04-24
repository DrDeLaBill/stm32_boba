/* Copyright © 2024 Georgy E. All rights reserved. */

#include "UI.h"

#include <cstdio>
#include <cstring>

#include "bmp.h"
#include "log.h"
#include "soul.h"
#include "main.h"
#include "sensor.h"
#include "bmacro.h"
#include "display.h"
#include "settings.h"
#include "hal_defs.h"


utl::circle_buffer<UI::UI_CLICKS_SIZE, UI::button_t> UI::clicks;
Button UI::buttons[UI::BUTTONS_COUNT] = {
	{BTN_UP_GPIO_Port,    BTN_UP_Pin,    true},
	{BTN_DOWN_GPIO_Port,  BTN_DOWN_Pin,  true},
	{BTN_MODE_GPIO_Port,  BTN_MODE_Pin,  true},
	{BTN_ENTER_GPIO_Port, BTN_ENTER_Pin, true},
//	{BTN_F1_GPIO_Port,    BTN_F1_Pin, true},
//	{BTN_F2_GPIO_Port,    BTN_F2_Pin, true},
//	{BTN_F3_GPIO_Port,    BTN_F3_Pin, true}
};

utl::Timer UI::timer(SECOND_MS);
fsm::FiniteStateMachine<UI::fsm_table> UI::fsm;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin) {
	case BTN_UP_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "UP irq %u", HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin));
#endif
		UI::buttons[UI::BUTTON_UP].tick();
		if (UI::buttons[UI::BUTTON_UP].wasClicked()) {
			UI::clicks.push_back(UI::BUTTON_UP);
			set_status(MANUAL_NEED_VALVE_UP);
		} else {
			reset_status(MANUAL_NEED_VALVE_UP);
		}
		break;
	case BTN_DOWN_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "DOWN irq %u", HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin));
#endif
		UI::buttons[UI::BUTTON_DOWN].tick();
		if (UI::buttons[UI::BUTTON_DOWN].wasClicked()) {
			UI::clicks.push_back(UI::BUTTON_DOWN);
			set_status(MANUAL_NEED_VALVE_DOWN);
		} else {
			reset_status(MANUAL_NEED_VALVE_DOWN);
		}
		break;
	case BTN_MODE_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "MODE irq %u", HAL_GPIO_ReadPin(BTN_MODE_GPIO_Port, BTN_MODE_Pin));
#endif
		UI::buttons[UI::BUTTON_MODE].tick();
		if (UI::buttons[UI::BUTTON_MODE].wasClicked()) {
			UI::clicks.push_back(UI::BUTTON_MODE);
		}
		break;
	case BTN_ENTER_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "ENTER irq %u", HAL_GPIO_ReadPin(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin));
#endif
		UI::buttons[UI::BUTTON_ENTER].tick();
		if (UI::buttons[UI::BUTTON_ENTER].wasClicked()) {
			UI::clicks.push_back(UI::BUTTON_ENTER);
		}
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown button interrupt");
#endif
		break;
	};
}


void UI::tick()
{
	fsm.proccess();
}

void UI::showHeader()
{
	display_set_color(DISPLAY_COLOR_WHITE);
	display_text_show(
		display_width() / 2,
		DISPLAY_HEADER_HEIGHT / 2,
		&Font24,
		DISPLAY_ALIGN_CENTER,
		"statusbar",
		strlen("statusbar")
	);
}

void UI::showFooter()
{
	uint16_t halfSection = display_width() / 3 / 2;

	display_set_color(DISPLAY_COLOR_WHITE);
	display_text_show(
		halfSection,
		DISPLAY_HEADER_HEIGHT + DISPLAY_CONTENT_HEIGHT + (DISPLAY_FOOTER_HEIGHT / 2),
		&Font24,
		DISPLAY_ALIGN_CENTER,
		"F1",
		strlen("F1")
	);

	display_set_color(DISPLAY_COLOR_WHITE);
	display_text_show(
		display_width() / 3  + halfSection,
		DISPLAY_HEADER_HEIGHT + DISPLAY_CONTENT_HEIGHT + (DISPLAY_FOOTER_HEIGHT / 2),
		&Font24,
		DISPLAY_ALIGN_CENTER,
		"F2",
		strlen("F2")
	);

	display_set_color(DISPLAY_COLOR_WHITE);
	display_text_show(
		2 * display_width() / 3  + halfSection,
		DISPLAY_HEADER_HEIGHT + DISPLAY_CONTENT_HEIGHT + (DISPLAY_FOOTER_HEIGHT / 2),
		&Font24,
		DISPLAY_ALIGN_CENTER,
		"F3",
		strlen("F3")
	);
}

void UI::showValue()
{
	uint16_t offset_x = display_width() / 2;
	uint16_t offset_y = display_height() / (uint16_t)2 - Font16.Height - DEFAULT_MARGIN; // TODO

	{
		char target[30] = {};
		snprintf(target, sizeof(target), "Target: %03lu.%02lu", settings.last_target / 100, settings.last_target % 100);
		display_set_color(DISPLAY_COLOR_WHITE);
		display_text_show(
			offset_x,
			offset_y,
			&Font16,
			DISPLAY_ALIGN_CENTER,
			target,
			strlen(target)
		);
	}

	{
		offset_y = display_height() / 2;
		char value[30] = {};
		snprintf(value, sizeof(value), "Value: %03lu.%02lu", get_sensor_value() / 100, get_sensor_value() % 100);
		display_set_color(DISPLAY_COLOR_WHITE);
		display_text_show(
			offset_x,
			offset_y,
			&Font24,
			DISPLAY_ALIGN_CENTER,
			value,
			strlen(value)
		);
	}
}

void UI::showUp(bool flag)
{
	extern const BITMAPSTRUCT bmp_up_15x15;
	extern const BITMAPSTRUCT bmp_down_15x15;

	uint16_t x = DEFAULT_MARGIN;
	uint16_t y =
		DISPLAY_HEADER_HEIGHT +
		DISPLAY_CONTENT_HEIGHT -
		DEFAULT_MARGIN -
		static_cast<uint16_t>(bmp_down_15x15.infoHeader.biHeight) -
		(uint16_t)DEFAULT_MARGIN - // TODO
		static_cast<uint16_t>(bmp_up_15x15.infoHeader.biHeight)
	;

	if (flag) {
		display_draw_bitmap(x, y, &bmp_up_15x15);
	} else {
		display_clear_rect(x, y, static_cast<uint16_t>(bmp_up_15x15.infoHeader.biWidth), static_cast<uint16_t>(bmp_up_15x15.infoHeader.biHeight));
	}
	HAL_GPIO_WritePin(LED_UP_GPIO_Port, LED_UP_Pin, static_cast<GPIO_PinState>(flag));
}

void UI::showDown(bool flag)
{
	extern const BITMAPSTRUCT bmp_down_15x15;

	uint16_t x = DEFAULT_MARGIN;
	uint16_t y =
		DISPLAY_HEADER_HEIGHT +
		DISPLAY_CONTENT_HEIGHT -
		DEFAULT_MARGIN -
		static_cast<uint16_t>(bmp_down_15x15.infoHeader.biHeight)
	;

	if (flag) {
		display_draw_bitmap(x, y, &bmp_down_15x15);
	} else {
		display_clear_rect(x, y, static_cast<uint16_t>(bmp_down_15x15.infoHeader.biWidth), static_cast<uint16_t>(bmp_down_15x15.infoHeader.biHeight));
	}
	HAL_GPIO_WritePin(LED_DOWN_GPIO_Port, LED_DOWN_Pin, static_cast<GPIO_PinState>(flag));
}

void UI::_init_s::operator ()() const
{
    display_init();
	display_clear();
	display_set_color(DISPLAY_COLOR_WHITE);
	display_text_show(
		display_width() / 2,
		display_height() / 2,
		&Font24,
		DISPLAY_ALIGN_CENTER,
		"bObA",
		4
	);

	fsm.push_event(success_e{});
}


#define LOAD_POINT_COUNT (3)
void UI::_load_s::operator ()() const
{
	static unsigned counter = 0;

	if (!is_status(WAIT_LOAD)) {
		fsm.push_event(success_e{});
	}
	if (is_status(NO_SENSOR)) {
		fsm.push_event(no_sens_e{});
	}

	if (timer.wait()) {
		return;
	}
	timer.start();

	if (counter > LOAD_POINT_COUNT) {
		counter = 0;
	}
	char label[20] = "Loading";
	for (unsigned i = 0; i < LOAD_POINT_COUNT; i++) {
		if (i < counter) {
			label[strlen(label)] = '.';
		} else {
			label[strlen(label)] = ' ';
		}
	}
	display_set_color(DISPLAY_COLOR_WHITE);
	display_text_show(
		display_width() / 2,
		display_height() / 2,
		&Font24,
		DISPLAY_ALIGN_CENTER,
		label,
		strlen(label)
	);

	counter++;
}


void UI::_no_sens_s::operator ()() const
{
	showHeader();
	showFooter();

	char label[] = "NO SENSOR";
	display_set_color(DISPLAY_COLOR_WHITE);
	display_text_show(
		display_width() / 2,
		display_height() / 2,
		&Font24,
		DISPLAY_ALIGN_CENTER,
		label,
		strlen(label)
	);

	if (!is_status(NO_SENSOR)) {
	}
}


void UI::_manual_mode_s::operator ()() const
{
	showValue();
	showUp(is_status(MANUAL_NEED_VALVE_UP));
	showDown(is_status(MANUAL_NEED_VALVE_DOWN));

	HAL_GPIO_WritePin(LED_CENTER_GPIO_Port, LED_CENTER_Pin, static_cast<GPIO_PinState>(settings.last_target == get_sensor_value()));

	if (clicks.empty()) {
		return;
	}

	button_t btn = clicks.pop_front();
	switch (btn) {
	case BUTTON_MODE:
		fsm.push_event(change_mode_e{});
		break;
	case BUTTON_ENTER:
		settings.last_target = get_sensor_value();
		set_status(NEED_SAVE_SETTINGS);
		break;
	case BUTTON_UP:
		break;
	case BUTTON_DOWN:
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown button in buffer");
#endif
		break;
	}
}

void UI::_auto_mode_s::operator ()() const
{
	showValue();

	HAL_GPIO_WritePin(LED_CENTER_GPIO_Port, LED_CENTER_Pin, static_cast<GPIO_PinState>(settings.last_target == get_sensor_value()));

	if (clicks.empty()) {
		return;
	}

	button_t btn = clicks.pop_front();
	switch (btn) {
	case BUTTON_MODE:
		fsm.push_event(change_mode_e{});
		break;
	case BUTTON_ENTER:
		break;
	case BUTTON_UP:
		break;
	case BUTTON_DOWN:
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown button in buffer");
#endif
		break;
	}
}


void UI::_error_s::operator ()() const
{
	display_set_color(DISPLAY_COLOR_RED);
	display_text_show(
		display_width() / 2,
		display_height() / 2,
		&Font24,
		DISPLAY_ALIGN_CENTER,
		"ERROR",
		strlen("ERROR")
	);
}

void UI::none_a::operator ()() const
{
	display_clear_content();
}

void UI::load_start_a::operator ()() const
{
	display_clear();
	display_sections_show();
	timer.changeDelay(SECOND_MS);

	showHeader();
	showFooter();
}

void UI::no_sens_start_a::operator ()() const
{
	display_clear();
	display_sections_show();
}

void UI::manual_start_a::operator ()() const
{
	display_clear();
	display_sections_show();

	showHeader();
	showFooter();
}

void UI::auto_start_a::operator ()() const
{
	display_clear();
	display_sections_show();

	showHeader();
	showFooter();
}
