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

#include "App.h"


#define LOAD_POINT_COUNT (3)


utl::circle_buffer<UI::UI_CLICKS_SIZE, uint16_t> UI::clicks;
std::pair<uint16_t, Button> UI::buttons[UI::BUTTONS_COUNT] = {
	std::make_pair<uint16_t, Button>(BTN_F1_Pin,    {BTN_F1_GPIO_Port,    BTN_F1_Pin,    true}),
	std::make_pair<uint16_t, Button>(BTN_DOWN_Pin,  {BTN_DOWN_GPIO_Port,  BTN_DOWN_Pin,  true}),
	std::make_pair<uint16_t, Button>(BTN_UP_Pin,    {BTN_UP_GPIO_Port,    BTN_UP_Pin,    true}),
	std::make_pair<uint16_t, Button>(BTN_ENTER_Pin, {BTN_ENTER_GPIO_Port, BTN_ENTER_Pin, true}),
	std::make_pair<uint16_t, Button>(BTN_MODE_Pin,  {BTN_MODE_GPIO_Port,  BTN_MODE_Pin,  true}),
	std::make_pair<uint16_t, Button>(BTN_F2_Pin,    {BTN_F2_GPIO_Port,    BTN_F2_Pin,    true}),
	std::make_pair<uint16_t, Button>(BTN_F3_Pin,    {BTN_F3_GPIO_Port,    BTN_F3_Pin,    true})
};

utl::Timer UI::timer(SECOND_MS);
fsm::FiniteStateMachine<UI::fsm_table> UI::fsm;
std::unique_ptr<Menu> UI::serviceMenu;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	std::pair<uint16_t, Button>* pair = NULL;
	for (unsigned i = 0; i < __arr_len(UI::buttons); i++) {
		if (UI::buttons[i].first == GPIO_Pin) {
			pair = &(UI::buttons[i]);
			break;
		}
	}
	if (!pair) {
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown button interrupt");
#endif
		return;
	}
	pair->second.tick();
	bool wasClicked = pair->second.wasClicked();
	if (wasClicked) {
		UI::clicks.push_back(GPIO_Pin);
	}
	switch (GPIO_Pin) {
	case BTN_UP_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "UP irq %u", HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin));
#endif
		if (wasClicked) {
			set_status(MANUAL_NEED_VALVE_UP);
		} else {
			reset_status(MANUAL_NEED_VALVE_UP);
		}
		break;
	case BTN_DOWN_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "DOWN irq %u", HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin));
#endif
		if (wasClicked) {
			set_status(MANUAL_NEED_VALVE_DOWN);
		} else {
			reset_status(MANUAL_NEED_VALVE_DOWN);
		}
		break;
	case BTN_MODE_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "MODE irq %u", HAL_GPIO_ReadPin(BTN_MODE_GPIO_Port, BTN_MODE_Pin));
#endif
		break;
	case BTN_ENTER_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "ENTER irq %u", HAL_GPIO_ReadPin(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin));
#endif
		break;
	case BTN_F1_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "F1 irq %u", HAL_GPIO_ReadPin(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin));
#endif
		if (wasClicked) {
			set_status(BTN_F1_PRESSED);
		} else {
			reset_status(BTN_F1_PRESSED);
		}
		break;
	case BTN_F2_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "F2 irq %u", HAL_GPIO_ReadPin(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin));
#endif
		if (wasClicked) {
			set_status(BTN_F2_PRESSED);
		} else {
			reset_status(BTN_F2_PRESSED);
		}
		break;
	case BTN_F3_Pin:
#ifdef DEBUG
		printTagLog(UI::TAG, "F3 irq %u", HAL_GPIO_ReadPin(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin));
#endif
		if (wasClicked) {
			set_status(BTN_F3_PRESSED);
		} else {
			reset_status(BTN_F3_PRESSED);
		}
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown button interrupt");
#endif
		break;
	};
}


UI::UI()
{
	MenuItem menuItems[] = {
		{"PID Kp:",        "0.00"},
		{"PID Ki:",        "0.00"},
		{"PID Kd:",        "0.00"},
		{"PID sambpling:", "0"}
	};
	serviceMenu = std::make_unique<Menu>(menuItems, __arr_len(menuItems));
}

void UI::tick()
{
	fsm.proccess();
}

void UI::showHeader()
{
//	display_set_color(DISPLAY_COLOR_WHITE);
//	display_text_show(
//		display_width() / 2,
//		DISPLAY_HEADER_HEIGHT / 2,
//		&Font24,
//		DISPLAY_ALIGN_CENTER,
//		"statusbar",
//		strlen("statusbar")
//	);
}

void UI::showFooter()
{
	static uint16_t f1_color = DISPLAY_COLOR_WHITE;
	static uint16_t f2_color = DISPLAY_COLOR_WHITE;
	static uint16_t f3_color = DISPLAY_COLOR_WHITE;

	uint16_t halfSection = display_width() / 3 / 2;

	uint16_t x = 0;
	uint16_t y = DISPLAY_HEADER_HEIGHT + DISPLAY_CONTENT_HEIGHT + 1;
	uint16_t w = display_width() / 3;
	uint16_t h = display_height() - y;
	uint16_t curr_color = is_status(BTN_F1_PRESSED) ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (f1_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f1_color = curr_color;
	}
	if (!is_status(BTN_F1_PRESSED)) {
		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			x + halfSection,
			y + (DISPLAY_FOOTER_HEIGHT / 2),
			&Font24,
			DISPLAY_ALIGN_CENTER,
			"F1",
			strlen("F1")
		);
	}


	w -= 1;
	x += display_width() / 3 + 1;
	curr_color = is_status(BTN_F2_PRESSED) ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (f2_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f2_color = curr_color;
	}
	if (!is_status(BTN_F2_PRESSED)) {
		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			x  + halfSection,
			y + (DISPLAY_FOOTER_HEIGHT / 2),
			&Font24,
			DISPLAY_ALIGN_CENTER,
			"F2",
			strlen("F2")
		);
	}


	x += display_width() / 3;
	curr_color = is_status(BTN_F3_PRESSED) ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (f3_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f3_color = curr_color;
	}
	if (!is_status(BTN_F3_PRESSED)) {
		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			x + halfSection,
			y + (DISPLAY_FOOTER_HEIGHT / 2),
			&Font24,
			DISPLAY_ALIGN_CENTER,
			"F3",
			strlen("F3")
		);
	}
}

void UI::showValue()
{
	uint16_t offset_x = display_width() / 2;
	uint16_t offset_y = display_height() / (uint16_t)2 - Font16.Height - DEFAULT_MARGIN; // TODO

	{
		char target[30] = {};
		snprintf(target, sizeof(target), "Target: %03d.%02d", settings.last_target / 100, __abs(settings.last_target % 100));
		display_set_color(DISPLAY_COLOR_BLACK);
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
		snprintf(value, sizeof(value), "Value: %03d.%02d", get_sensor_value() / 100, __abs(get_sensor_value() % 100));
		display_set_color(DISPLAY_COLOR_BLACK);
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

void UI::showLoading()
{
	static unsigned counter = 0;
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
	display_set_color(DISPLAY_COLOR_BLACK);
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

bool UI::isServiceCombination()
{
	if (clicks.count() < 4) {
		return false;
	}

	bool flag = true;
	for (unsigned i = 0; i < clicks.count() - 3; i++) {
		if (clicks[i]     == BTN_F1_Pin &&
			clicks[i + 1] == BTN_F3_Pin &&
			clicks[i + 2] == BTN_F3_Pin &&
			clicks[i + 3] == BTN_F1_Pin
		) {
			return true;
		}
	}

	return false;
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

void UI::showMiddle(bool flag)
{
	GPIO_PinState enable_mid = static_cast<GPIO_PinState>(flag);
	HAL_GPIO_WritePin(LED_MID_GPIO_Port, LED_MID_Pin, enable_mid);

	GPIO_PinState enable_center = static_cast<GPIO_PinState>(
		HAL_GPIO_ReadPin(LED_UP_GPIO_Port, LED_UP_Pin) ||
		HAL_GPIO_ReadPin(LED_MID_GPIO_Port, LED_MID_Pin) ||
		HAL_GPIO_ReadPin(LED_DOWN_GPIO_Port, LED_DOWN_Pin)
	);
	HAL_GPIO_WritePin(LED_CENTER_GPIO_Port, LED_CENTER_Pin, enable_center);
}

void UI::_init_s::operator ()() const
{
    display_init();
	display_clear();
	display_set_color(DISPLAY_COLOR_BLACK);
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


void UI::_load_s::operator ()() const
{
	if (!is_status(WAIT_LOAD)) {
		fsm.push_event(success_e{});
	}
	if (is_status(NO_SENSOR)) {
		fsm.push_event(no_sens_e{});
	}
	if (has_errors()) {
		fsm.push_event(error_e{});
	}

	if (timer.wait()) {
		return;
	}
	timer.start();

	showLoading();
}


void UI::_no_sens_s::operator ()() const
{
	showHeader();
	showFooter();

	char label[] = "NO SENSOR";
	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		display_height() / 2,
		&Font24,
		DISPLAY_ALIGN_CENTER,
		label,
		strlen(label)
	);

	if (!is_status(NO_SENSOR)) {
		fsm.push_event(sens_found_e{});
	}

	if (isServiceCombination()) {
		fsm.push_event(service_e{});
	}
}


void UI::_manual_mode_s::operator ()() const
{
	showValue();
	showUp(is_status(MANUAL_NEED_VALVE_UP));
	showDown(is_status(MANUAL_NEED_VALVE_DOWN));
	showMiddle(settings.last_target == get_sensor_value());

	if (is_status(NO_SENSOR)) {
		fsm.push_event(no_sens_e{});
	}
	if (has_errors()) {
		fsm.push_event(error_e{});
	}

	if (isServiceCombination()) {
		fsm.push_event(service_e{});
	}

	if (clicks.empty()) {
		return;
	}

	uint16_t click = clicks.pop_front();
	switch (click) {
	case BTN_MODE_Pin:
		fsm.push_event(change_mode_e{});
		App::setMode(APP_MODE_SURFACE);
		break;
	case BTN_ENTER_Pin:
		settings.last_target = get_sensor_value();
		set_status(NEED_SAVE_SETTINGS);
		break;
	case BTN_UP_Pin:
	case BTN_DOWN_Pin:
	case BTN_F1_Pin:
	case BTN_F2_Pin:
	case BTN_F3_Pin:
		clicks.push_back(click);
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown button in buffer");
#endif
		fsm.push_event(error_e{});
		set_error(INTERNAL_ERROR);
		break;
	}
}

void UI::_auto_mode_s::operator ()() const
{
	showValue();
	showUp(is_status(AUTO_NEED_VALVE_UP));
	showDown(is_status(AUTO_NEED_VALVE_DOWN));
	showMiddle(settings.last_target == get_sensor_value());

	if (is_status(NO_SENSOR)) {
		fsm.push_event(no_sens_e{});
	}
	if (has_errors()) {
		fsm.push_event(error_e{});
	}

	if (clicks.empty()) {
		return;
	}

	switch (clicks.pop_front()) {
	case BTN_MODE_Pin:
		fsm.push_event(change_mode_e{});
		App::setMode(APP_MODE_MANUAL);
		break;
	case BTN_ENTER_Pin:
		break;
	case BTN_UP_Pin:
		break;
	case BTN_DOWN_Pin:
		break;
	case BTN_F1_Pin:
		App::setMode(APP_MODE_GROUND);
		break;
	case BTN_F2_Pin:
		App::setMode(APP_MODE_STRING);
		break;
	case BTN_F3_Pin:
		App::setMode(APP_MODE_SURFACE);
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown button in buffer");
#endif
		fsm.push_event(error_e{});
		set_error(INTERNAL_ERROR);
		break;
	}
}


void UI::_service_s::operator ()() const
{
	showFooter();

	char value[MenuItem::VALUE_MAX_LEN] = "";
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.kp), __abs(((int)(settings.kp * 100)) % 100));
	serviceMenu->setValue(0, value);

	memset(value, 0, sizeof(value));
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.ki), __abs(((int)(settings.ki * 100)) % 100));
	serviceMenu->setValue(1, value);

	memset(value, 0, sizeof(value));
	snprintf(value, sizeof(value), "%d.%02d", ((int)settings.kd), __abs(((int)(settings.kd * 100)) % 100));
	serviceMenu->setValue(2, value);

	memset(value, 0, sizeof(value));
	snprintf(value, sizeof(value), "%lu ms", settings.sampling);
	serviceMenu->setValue(3, value);

	if (!clicks.empty()) {
		serviceMenu->click(clicks.pop_front());
	}

	serviceMenu->show();
}


void UI::_error_s::operator ()() const
{
	char line[20] = "";
	unsigned error = get_first_error();
	if (error) {
		snprintf(line, sizeof(line), "ERROR%u", error);

		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			display_width() / 2,
			display_height() / 2,
			&Font24,
			DISPLAY_ALIGN_CENTER,
			line,
			strlen(line)
		);
	} else {
		fsm.push_event(success_e{});
	}
}

void UI::error_a::operator ()() const
{
	display_clear_content();

	showDown(false);
	showUp(false);
	showMiddle(false);
}

#define LOADING_DELAY_MS ((uint32_t)300)
void UI::load_start_a::operator ()() const
{
	display_clear();
	display_sections_show();
	timer.changeDelay(LOADING_DELAY_MS);

	showHeader();
	showFooter();
	showLoading();
}

void UI::no_sens_start_a::operator ()() const
{
	clicks.clear();
	fsm.clear_events();

	display_clear();
	display_sections_show();

	showDown(false);
	showUp(false);
	showMiddle(false);
}

void UI::manual_start_a::operator ()() const
{
	clicks.clear();
	fsm.clear_events();

	display_sections_show();

	char line[] = "    manual    ";
	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		DISPLAY_HEADER_HEIGHT / 2,
		&Font24,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
	);

	showHeader();
	showFooter();
}

void UI::auto_start_a::operator ()() const
{
	clicks.clear();
	fsm.clear_events();

	display_sections_show();

	char line[] = "     auto     ";
	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		DISPLAY_HEADER_HEIGHT / 2,
		&Font24,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
	);

	showHeader();
	showFooter();
}

void UI::service_start_a::operator ()() const
{
	clicks.clear();
	fsm.clear_events();

	display_clear_content();
	display_sections_show();

	char line[] = "    service    ";
	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		DISPLAY_HEADER_HEIGHT / 2,
		&Font24,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
	);
}
