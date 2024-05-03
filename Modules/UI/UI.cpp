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
#include "Callbacks.h"


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
MenuItem menuItems[] =
{
	{(new version_callback()),          false, "Version:",       "v0.0.0"},
	{(new label_callback())  ,          false, "        SURFACE MODE        "},
	{(new surface_kp_callback()),       true,  "PID Kp:",        "0.00"},
	{(new surface_ki_callback()),       true,  "PID Ki:",        "0.00"},
	{(new surface_kd_callback()),       true,  "PID Kd:",        "0.00"},
	{(new surface_sampling_callback()), true,  "PID sampling:",  "0 ms"},
	{(new label_callback())  ,          false, "        GROUND  MODE        "},
	{(new ground_kp_callback()),        true,  "PID Kp:",        "0.00"},
	{(new ground_ki_callback()),        true,  "PID Ki:",        "0.00"},
	{(new ground_kd_callback()),        true,  "PID Kd:",        "0.00"},
	{(new ground_sampling_callback()),  true,  "PID sampling:",  "0 ms"},
	{(new label_callback())  ,          false, "        STRING  MODE        "},
	{(new string_kp_callback()),        true,  "PID Kp:",        "0.00"},
	{(new string_ki_callback()),        true,  "PID Ki:",        "0.00"},
	{(new string_kd_callback()),        true,  "PID Kd:",        "0.00"},
	{(new string_sampling_callback()),  true,  "PID sampling:",  "0 ms"}
};
std::unique_ptr<Menu> UI::serviceMenu = std::make_unique<Menu>(
	0,
	DISPLAY_HEADER_HEIGHT,
	display_width(),
	DISPLAY_CONTENT_HEIGHT,
	menuItems,
	__arr_len(menuItems)
);


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
#if UI_BEDUG
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
#if UI_BEDUG
		printTagLog(UI::TAG, "UP irq %u", HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin));
#endif
		if (wasClicked) {
			set_status(MANUAL_NEED_VALVE_UP);
		} else {
			reset_status(MANUAL_NEED_VALVE_UP);
		}
		break;
	case BTN_DOWN_Pin:
#if UI_BEDUG
		printTagLog(UI::TAG, "DOWN irq %u", HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin));
#endif
		if (wasClicked) {
			set_status(MANUAL_NEED_VALVE_DOWN);
		} else {
			reset_status(MANUAL_NEED_VALVE_DOWN);
		}
		break;
	case BTN_MODE_Pin:
#if UI_BEDUG
		printTagLog(UI::TAG, "MODE irq %u", HAL_GPIO_ReadPin(BTN_MODE_GPIO_Port, BTN_MODE_Pin));
#endif
		break;
	case BTN_ENTER_Pin:
#if UI_BEDUG
		printTagLog(UI::TAG, "ENTER irq %u", HAL_GPIO_ReadPin(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin));
#endif
		break;
	case BTN_F1_Pin:
#if UI_BEDUG
		printTagLog(UI::TAG, "F1 irq %u", HAL_GPIO_ReadPin(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin));
#endif
		if (wasClicked) {
			set_status(BTN_F1_PRESSED);
		} else {
			reset_status(BTN_F1_PRESSED);
		}
		break;
	case BTN_F2_Pin:
#if UI_BEDUG
		printTagLog(UI::TAG, "F2 irq %u", HAL_GPIO_ReadPin(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin));
#endif
		if (wasClicked) {
			set_status(BTN_F2_PRESSED);
		} else {
			reset_status(BTN_F2_PRESSED);
		}
		break;
	case BTN_F3_Pin:
#if UI_BEDUG
		printTagLog(UI::TAG, "F3 irq %u", HAL_GPIO_ReadPin(BTN_ENTER_GPIO_Port, BTN_ENTER_Pin));
#endif
		if (wasClicked) {
			set_status(BTN_F3_PRESSED);
		} else {
			reset_status(BTN_F3_PRESSED);
		}
		break;
	default:
#if UI_BEDUG
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
{}

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
	char linef1[] = "F1";
	display_set_background(curr_color);
	display_set_color(is_status(BTN_F1_PRESSED) ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		&Font24,
		DISPLAY_ALIGN_CENTER,
		linef1,
		strlen(linef1)
	);


	w -= 1;
	x += static_cast<uint16_t>(display_width() / 3 + 1);
	curr_color = is_status(BTN_F2_PRESSED) ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (f2_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f2_color = curr_color;
	}
	char linef2[] = "F2";
	display_set_background(curr_color);
	display_set_color(is_status(BTN_F1_PRESSED) ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		&Font24,
		DISPLAY_ALIGN_CENTER,
		linef2,
		strlen(linef2)
	);


	x += static_cast<uint16_t>(display_width() / 3);
	curr_color = is_status(BTN_F3_PRESSED) ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (f3_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f3_color = curr_color;
	}
	char linef3[] = "F3";
	display_set_background(curr_color);
	display_set_color(is_status(BTN_F1_PRESSED) ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		&Font24,
		DISPLAY_ALIGN_CENTER,
		linef3,
		strlen(linef3)
	);
}

void UI::showValue()
{
	uint16_t offset_x = display_width() / 2;
	uint16_t offset_y = static_cast<uint16_t>(display_height() / (uint16_t)2 - Font16.Height - DEFAULT_MARGIN);

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

	for (uint8_t i = 0; i < clicks.count() - 3; i++) {
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
	uint16_t y = static_cast<uint16_t>(
		DISPLAY_HEADER_HEIGHT +
		DISPLAY_CONTENT_HEIGHT -
		DEFAULT_MARGIN -
		bmp_down_15x15.infoHeader.biHeight -
		DEFAULT_MARGIN -
		bmp_up_15x15.infoHeader.biHeight
	);

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

	if (!clicks.empty()) {
		serviceMenu->click(clicks.pop_front());
	}

	if (is_status(NEED_UI_EXIT)) {
		reset_status(NEED_UI_EXIT);
		fsm.push_event(service_e{});
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

	serviceMenu->reset();
}
