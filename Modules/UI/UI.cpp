/* Copyright © 2024 Georgy E. All rights reserved. */

#include "UI.h"

#include <cstdio>
#include <cstring>

#include "bmp.h"
#include "log.h"
#include "soul.h"
#include "main.h"
#include "utils.h"
#include "sensor.h"
#include "bmacro.h"
#include "display.h"
#include "settings.h"
#include "hal_defs.h"

#include "App.h"
#include "Callbacks.h"
#include "CodeStopwatch.h"


#define LOAD_POINT_COUNT   (3)
#define PHRASE_LEN_MAX     (40)

#define ADD_SPACES_STRING(PHRASE, FONT) \
	char NAME[PHRASE_LEN_MAX] = {}; \
	memset(NAME, CHAR, __min(DISPLAY_WIDTH / FONT.Width, sizeof(target) - 1)); \
	NAME[__min(strlen(NAME), sizeof(NAME) - 1)] = 0; \



utl::circle_buffer<UI::UI_CLICKS_SIZE, uint16_t> UI::clicks;
std::unordered_map<uint16_t, Button> UI::buttons = {
	{BTN_F1_Pin,    {BTN_F1_GPIO_Port,    BTN_F1_Pin,    true}},
	{BTN_DOWN_Pin,  {BTN_DOWN_GPIO_Port,  BTN_DOWN_Pin,  true}},
	{BTN_UP_Pin,    {BTN_UP_GPIO_Port,    BTN_UP_Pin,    true}},
	{BTN_ENTER_Pin, {BTN_ENTER_GPIO_Port, BTN_ENTER_Pin, true}},
	{BTN_MODE_Pin,  {BTN_MODE_GPIO_Port,  BTN_MODE_Pin,  true}},
	{BTN_F2_Pin,    {BTN_F2_GPIO_Port,    BTN_F2_Pin,    true}},
	{BTN_F3_Pin,    {BTN_F3_GPIO_Port,    BTN_F3_Pin,    true}}
};
utl::Timer UI::timer(SECOND_MS);
fsm::FiniteStateMachine<UI::fsm_table> UI::fsm;
MenuItem menuItems[] =
{
	{(new version_callback()),          false, "Version:",       "v0.0.0"},
	{(new language_callback()),         true,  "Language:",      "_"},
	{(new label_callback())  ,          false, "        SURFACE MODE        "},
	{(new surface_kp_callback()),       true,  "PID Kp:",        "0.00"},
	{(new surface_ki_callback()),       true,  "PID Ki:",        "0.00"},
	{(new surface_kd_callback()),       true,  "PID Kd:",        "0.00"},
	{(new surface_sampling_callback()), true,  "PID sampling:",  "0 ms"},
	{(new label_callback())  ,          false, "        GROUND  MODE        "},
	{(new bigsky_kp_callback()),        true,  "PID Kp:",        "0.00"},
	{(new bigsky_ki_callback()),        true,  "PID Ki:",        "0.00"},
	{(new bigsky_kd_callback()),        true,  "PID Kd:",        "0.00"},
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
SENSOR_MODE UI::manual_f1_mode = SENSOR_MODE_SURFACE;
SENSOR_MODE UI::manual_f3_mode = SENSOR_MODE_STRING;


void UI::tick()
{
	utl::CodeStopwatch watch("UI2", 100);
	fsm.proccess();
}


void UI::buttonsTick()
{
	utl::CodeStopwatch watch("UI1", 100);

	for (auto& button : buttons) {
		button.second.tick();
		if (button.second.oneClick()) {
			clicks.push_back(button.first);
		}
	}
}

void UI::showMode()
{
	sFONT* bitmap = NULL;
	switch (get_sensor_mode()) {
	case SENSOR_MODE_SURFACE:
		bitmap = &surface_bitmap;
		break;
	case SENSOR_MODE_STRING:
		bitmap = &string_bitmap;
		break;
	case SENSOR_MODE_BIGSKY:
		bitmap = &bigsky_bitmap;
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown APP mode");
#endif
		fsm.push_event(error_e{});
		set_error(INTERNAL_ERROR);
		Error_Handler();
		return;
	};

	char line[] = " ";
	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		(uint16_t)(DISPLAY_HEADER_HEIGHT + bitmap->Height),
		bitmap,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
	);
}

void UI::showHeader()
{}

void UI::showAutoFooter()
{
	static uint16_t f1_color = DISPLAY_COLOR_WHITE;
	static uint16_t f2_color = DISPLAY_COLOR_WHITE;
	static uint16_t f3_color = DISPLAY_COLOR_WHITE;

	uint16_t halfSection = display_width() / 3 / 2;

	uint16_t x = 0;
	uint16_t y = DISPLAY_HEADER_HEIGHT + DISPLAY_CONTENT_HEIGHT + 1;
	uint16_t w = display_width() / 3;
	uint16_t h = display_height() - y;
	uint16_t curr_color = buttons[BTN_F1_Pin].pressed() ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	SENSOR_MODE mode = get_sensor_mode();
	char line[] = " ";

	if (mode == SENSOR_MODE_SURFACE) {
		curr_color = DISPLAY_COLOR_LIGHT_GRAY;
	}
	if (f1_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f1_color = curr_color;
	}
	display_set_background(curr_color);
	display_set_color(buttons[BTN_F1_Pin].pressed() ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		&surface_bitmap,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
	);


	w -= 1;
	x += static_cast<uint16_t>(display_width() / 3 + 1);
	curr_color = buttons[BTN_F2_Pin].pressed() ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (mode == SENSOR_MODE_STRING) {
		curr_color = DISPLAY_COLOR_LIGHT_GRAY;
	}
	if (f2_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f2_color = curr_color;
	}
	display_set_background(curr_color);
	display_set_color(buttons[BTN_F2_Pin].pressed() ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		&string_bitmap,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
	);


	x += static_cast<uint16_t>(display_width() / 3);
	curr_color = buttons[BTN_F3_Pin].pressed() ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (mode == SENSOR_MODE_BIGSKY) {
		curr_color = DISPLAY_COLOR_LIGHT_GRAY;
	}
	if (f3_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f3_color = curr_color;
	}
	display_set_background(curr_color);
	display_set_color(buttons[BTN_F3_Pin].pressed() ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		&bigsky_bitmap,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
	);
}

void UI::showManualFooter()
{
	static uint16_t f1_color = DISPLAY_COLOR_WHITE;
	static uint16_t f2_color = DISPLAY_COLOR_WHITE;
	static uint16_t f3_color = DISPLAY_COLOR_WHITE;

	uint16_t x = static_cast<uint16_t>(display_width() / 3 + 1);
	uint16_t y = DISPLAY_HEADER_HEIGHT + DISPLAY_CONTENT_HEIGHT + 1;
	uint16_t w = display_width() / 3 - 1;
	uint16_t h = display_height() - y;
	uint16_t halfSection = display_width() / 3 / 2;
	uint16_t curr_color = DISPLAY_COLOR_WHITE;

	curr_color = buttons[BTN_F2_Pin].pressed() ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (f2_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f2_color = curr_color;
	}
	char linef2[] = " ";
	display_set_background(curr_color);
	display_set_color(buttons[BTN_F2_Pin].pressed() ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		&settings_bitmap,
		DISPLAY_ALIGN_CENTER,
		linef2,
		strlen(linef2)
	);

	if (is_status(NO_SENSOR)) {
		return;
	}

	x = 0;
	w = display_width() / 3;
	manual_f1_mode = SENSOR_MODE_SURFACE;
	sFONT* bitmap = &surface_bitmap;
	if (manual_f1_mode == get_sensor_mode()) {
		manual_f1_mode = SENSOR_MODE_STRING;
		bitmap = &string_bitmap;
	}
	curr_color = buttons[BTN_F1_Pin].pressed() ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (f1_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f1_color = curr_color;
	}
	char linef1[] = " ";
	display_set_background(curr_color);
	display_set_color(buttons[BTN_F1_Pin].pressed() ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		bitmap,
		DISPLAY_ALIGN_CENTER,
		linef1,
		strlen(linef1)
	);

	w -= 1;
	x = static_cast<uint16_t>(display_width() / 3 * 2 + 1);
	manual_f3_mode = SENSOR_MODE_STRING;
	bitmap = &string_bitmap;
	if (manual_f1_mode == manual_f3_mode || manual_f3_mode == get_sensor_mode()) {
		manual_f3_mode = SENSOR_MODE_BIGSKY;
		bitmap = &bigsky_bitmap;
	}
	curr_color = buttons[BTN_F3_Pin].pressed() ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (f3_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f3_color = curr_color;
	}
	char linef3[] = " ";
	display_set_background(curr_color);
	display_set_color(buttons[BTN_F3_Pin].pressed() ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		bitmap,
		DISPLAY_ALIGN_CENTER,
		linef3,
		strlen(linef3)
	);
}

void UI::showServiceFooter()
{
	static uint16_t f1_color = DISPLAY_COLOR_WHITE;
	static uint16_t f3_color = DISPLAY_COLOR_WHITE;

	uint16_t halfSection = display_width() / 3 / 2;

	uint16_t x = 0;
	uint16_t y = DISPLAY_HEADER_HEIGHT + DISPLAY_CONTENT_HEIGHT + 1;
	uint16_t w = display_width() / 3;
	uint16_t h = display_height() - y;
	uint16_t curr_color = buttons[BTN_F1_Pin].pressed() ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (f1_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f1_color = curr_color;
	}
	char linef1[] = " ";
	display_set_background(curr_color);
	display_set_color(buttons[BTN_F1_Pin].pressed() ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		&back_bitmap,
		DISPLAY_ALIGN_CENTER,
		linef1,
		strlen(linef1)
	);

	w -= 1;
	x += static_cast<uint16_t>(display_width() / 3 + 1);
	display_fill_rect(x, y, w, h, DISPLAY_COLOR_WHITE);

	x += static_cast<uint16_t>(display_width() / 3);
	curr_color = buttons[BTN_F3_Pin].pressed() ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (f3_color != curr_color) {
		display_fill_rect(x, y, w, h, curr_color);
		f3_color = curr_color;
	}
	char linef3[] = " ";
	display_set_background(curr_color);
	display_set_color(buttons[BTN_F3_Pin].pressed() ? DISPLAY_COLOR_GRAY : DISPLAY_COLOR_BLACK);
	display_text_show(
		x + halfSection,
		y + (DISPLAY_FOOTER_HEIGHT / 2),
		&save_bitmap,
		DISPLAY_ALIGN_CENTER,
		linef3,
		strlen(linef3)
	);
}

void UI::showValue()
{
	uint16_t offset_x = display_width() / 2;
	uint16_t offset_y = static_cast<uint16_t>(display_height() / (uint16_t)2 - u8g2_font_8x13_t_cyrillic.Height - DEFAULT_MARGIN);

	{
		char target[PHRASE_LEN_MAX] = {};
		const char* phrase = t(T_TARGET, settings.language);
		snprintf(
			target,
			sizeof(target) - 1,
			"%s: %03d.%02d",
			phrase,
			settings.last_target / 100,
			__abs(settings.last_target % 100)
		);
		util_add_char(target, sizeof(target), ' ', (size_t)DISPLAY_WIDTH / u8g2_font_8x13_t_cyrillic.Width, ALIGN_MODE_CENTER);

		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			offset_x,
			offset_y,
			&u8g2_font_8x13_t_cyrillic,
			DISPLAY_ALIGN_CENTER,
			target,
			strlen(target)
		);
	}

	{
		offset_y = display_height() / 2;
		char value[PHRASE_LEN_MAX] = {};
		const char* phrase = t(T_VALUE, settings.language);
		snprintf(
			value,
			sizeof(value) - 1,
			"%s: %03d.%02d",
			phrase,
			get_sensor_average_value() / 100,
			__abs(get_sensor_average_value() % 100)
		);
		util_add_char(value, sizeof(value), ' ', (size_t)DISPLAY_WIDTH / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			offset_x,
			offset_y,
			&u8g2_font_10x20_t_cyrillic,
			DISPLAY_ALIGN_CENTER,
			value,
			strlen(value)
		);
	}
}

void UI::showLoading()
{
	char line[PHRASE_LEN_MAX] = {};
	const char* phrase = t(T_LOADING, settings.language);
	snprintf(line, sizeof(line) - 1, "%s", phrase);
	util_add_char(line, sizeof(line), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		display_height() / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
	);
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
	char line[PHRASE_LEN_MAX] = "bObA";
	const char* phrase = t(T_LOADING, settings.language);
	snprintf(line, sizeof(line) - 1, "%s", phrase);
	util_add_char(line, sizeof(line), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

    display_init();
	display_clear();
	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		display_height() / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
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
	showManualFooter();

	char line[PHRASE_LEN_MAX] = {};
	const char* phrase = t(T_NO_SENSOR, settings.language);
	snprintf(line, sizeof(line) - 1, "%s", phrase);
	util_add_char(line, sizeof(line), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		display_height() / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
	);

	if (!is_status(NO_SENSOR)) {
		fsm.push_event(sens_found_e{});
	}

	if (clicks.empty()) {
		return;
	}

	uint16_t click = clicks.pop_front();
	switch (click) {
	case BTN_F2_Pin:
		fsm.push_event(service_e{});
		break;
	case BTN_MODE_Pin:
	case BTN_ENTER_Pin:
	case BTN_UP_Pin:
	case BTN_DOWN_Pin:
	case BTN_F1_Pin:
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


void UI::_manual_mode_s::operator ()() const
{
	showMode();
	showManualFooter();
	showValue();
	showUp(is_status(MANUAL_NEED_VALVE_UP));
	showDown(is_status(MANUAL_NEED_VALVE_DOWN));
	showMiddle(__abs(get_sensor_average_value()) < TRIG_VALUE_LOW);

	if (is_status(NO_SENSOR)) {
		fsm.push_event(no_sens_e{});
	}
	if (has_errors()) {
		fsm.push_event(error_e{});
	}

	buttons[BTN_UP_Pin].pressed() ? set_status(MANUAL_NEED_VALVE_UP) : reset_status(MANUAL_NEED_VALVE_UP);
	buttons[BTN_DOWN_Pin].pressed() ? set_status(MANUAL_NEED_VALVE_DOWN) : reset_status(MANUAL_NEED_VALVE_DOWN);

	static bool target_reseted = false;
	if (buttons[BTN_ENTER_Pin].isHolded()) {
		settings.last_target = 0;
		target_reseted = true;
		set_status(NEED_SAVE_SETTINGS);
		return;
	}

	if (clicks.empty()) {
		return;
	}

	uint16_t click = clicks.pop_front();
	switch (click) {
	case BTN_MODE_Pin:
		fsm.push_event(change_mode_e{});
		App::setAppMode(APP_MODE_AUTO);
		break;
	case BTN_ENTER_Pin:
		if (target_reseted) {
			target_reseted = false;
			break;
		}
		settings.last_target += get_sensor_average_value();
		set_status(NEED_SAVE_SETTINGS);
		break;
	case BTN_F1_Pin:
		App::changeSensorMode(manual_f1_mode);
		break;
	case BTN_F2_Pin:
		fsm.push_event(service_e{});
		break;
	case BTN_F3_Pin:
		App::changeSensorMode(manual_f3_mode);
		break;
	case BTN_UP_Pin:
	case BTN_DOWN_Pin:
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
	showMode();
	showAutoFooter();
	showValue();
	showUp(is_status(AUTO_NEED_VALVE_UP));
	showDown(is_status(AUTO_NEED_VALVE_DOWN));
	showMiddle(__abs(get_sensor_average_value()) < TRIG_VALUE_LOW);

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
		App::setAppMode(APP_MODE_MANUAL);
		break;
	case BTN_ENTER_Pin:
		break;
	case BTN_UP_Pin:
		break;
	case BTN_DOWN_Pin:
		break;
	case BTN_F1_Pin:
		App::changeSensorMode(SENSOR_MODE_SURFACE);
		break;
	case BTN_F2_Pin:
		App::changeSensorMode(SENSOR_MODE_STRING);
		break;
	case BTN_F3_Pin:
		App::changeSensorMode(SENSOR_MODE_BIGSKY);
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
	showServiceFooter();

	auto button = buttons.find(BTN_UP_Pin);
	if (button != buttons.end()) {
		if (buttons[BTN_UP_Pin].isHolded()) {
			serviceMenu->holdUp();
		}
	}
	button = buttons.find(BTN_DOWN_Pin);
	if (button != buttons.end()) {
		if (buttons[BTN_DOWN_Pin].isHolded()) {
			serviceMenu->holdDown();
		}
	}

	if (!clicks.empty()) {
		serviceMenu->click(clicks.pop_front());
	}

	if (is_status(NEED_UI_EXIT)) {
		reset_status(NEED_UI_EXIT);
		is_status(NO_SENSOR) ?
			fsm.push_event(no_sens_e{}) :
			fsm.push_event(service_e{});
	}

	serviceMenu->show();
}


void UI::_error_s::operator ()() const
{
	unsigned error = get_first_error();

	if (error) {
		char line[PHRASE_LEN_MAX] = {};
		const char* phrase = t(T_ERROR, settings.language);
		snprintf(line, sizeof(line) - 1, "%s %u", phrase, error);
		util_add_char(line, sizeof(line), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);


		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			display_width() / 2,
			display_height() / 2,
			&u8g2_font_10x20_t_cyrillic,
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
	display_clear_header();
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

	display_clear_header();
	display_clear_content();
	display_clear_footer();
	display_sections_show();


	char mode[PHRASE_LEN_MAX] = {};
	const char* phrase1 = t(T_manual, settings.language);
	const char* phrase2 = t(T_mode, settings.language);
	snprintf(mode, sizeof(mode), "%s %s", phrase1, phrase2);
	util_add_char(mode, sizeof(mode), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		DISPLAY_HEADER_HEIGHT / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		mode,
		strlen(mode)
	);

	showHeader();
}

void UI::auto_start_a::operator ()() const
{
	clicks.clear();
	fsm.clear_events();

	display_clear_content();
	display_clear_footer();
	display_sections_show();

	char mode[PHRASE_LEN_MAX] = {};
	const char* phrase1 = t(T_auto, settings.language);
	const char* phrase2 = t(T_mode, settings.language);
	snprintf(mode, sizeof(mode), "%s %s", phrase1, phrase2);
	util_add_char(mode, sizeof(mode), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		DISPLAY_HEADER_HEIGHT / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		mode,
		strlen(mode)
	);

	showHeader();
}

void UI::service_start_a::operator ()() const
{
	clicks.clear();
	fsm.clear_events();

	display_clear_content();
	display_sections_show();

	char line[PHRASE_LEN_MAX] = {};
	const char* phrase1 = t(T_service, settings.language);
	const char* phrase2 = t(T_mode, settings.language);
	snprintf(line, sizeof(line), "%s %s", phrase1, phrase2);
	util_add_char(line, sizeof(line), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		DISPLAY_HEADER_HEIGHT / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line)
	);

	serviceMenu->reset();
}
