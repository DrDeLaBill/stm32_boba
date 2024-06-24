/* Copyright © 2024 Georgy E. All rights reserved. */

#include "UI.h"

#include <cstdio>
#include <cstring>

#include "bmp.h"
#include "glog.h"
#include "soul.h"
#include "main.h"
#include "gutils.h"
#include "sensor.h"
#include "bmacro.h"
#include "gstring.h"
#include "display.h"
#include "settings.h"
#include "hal_defs.h"

#include "App.h"
#include "Callbacks.h"
#include "CodeStopwatch.h"


#define DEFAULT_SCALE      (1)
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
	{(new version_callback()),          false},
	{(new language_callback()),         true},
	{(new surface_label_callback()),    false},
	{(new surface_snstv_callback()),    true},
	{(new surface_delay_callback()),    true},
	{(new string_label_callback()),     false},
	{(new string_snstv_callback()),     true},
	{(new string_delay_callback()),     true},
	{(new bigski_label_callback()),     false},
	{(new bigski_snstv_callback()),     true},
	{(new bigski_delay_callback()),     true},
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

uint16_t UI::f1_color = DISPLAY_COLOR_WHITE;
uint16_t UI::f2_color = DISPLAY_COLOR_WHITE;
uint16_t UI::f3_color = DISPLAY_COLOR_WHITE;

const char (*UI::loadStr)[TRANSLATE_MAX_LEN] = T_LOADING;


void UI::tick()
{
	utl::CodeStopwatch watch("UI2", 300);
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
	char sensors[PHRASE_LEN_MAX] = "";
	util_add_char(
		sensors,
		sizeof(sensors),
		' ',
		display_width() / u8g2_font_8x13_t_cyrillic.Width,
		ALIGN_MODE_CENTER
	);
	switch (get_sensor_target_mode()) {
	case SENSOR_MODE_SURFACE:
		bitmap = &surface_bitmap;
		break;
	case SENSOR_MODE_STRING:
		bitmap = &string_bitmap;
		break;
	case SENSOR_MODE_BIGSKI:
		bitmap = &bigski_bitmap;
		snprintf(
			sensors,
			sizeof(sensors) - 1,
			"%c%c%c",
			sensor2AB_available() ? '1' : '-',
			sensor2A7_available() ? '2' : '-',
			sensor2A8_available() ? '3' : '-'
		);
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

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		(uint16_t)(DISPLAY_HEADER_HEIGHT + u8g2_font_8x13_t_cyrillic.Height),
		&u8g2_font_8x13_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		sensors,
		strlen(sensors),
		DEFAULT_SCALE
	);

	char line[] = " ";
	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		(uint16_t)(DISPLAY_HEADER_HEIGHT + bitmap->Height),
		bitmap,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line),
		DEFAULT_SCALE
	);
}

void UI::showHeader()
{}

void UI::showServiceHeader()
{
	char line[PHRASE_LEN_MAX] = {};
	const char* phrase1 = t(T_SERVICE, settings.language);
	const char* phrase2 = t(T_MODE, settings.language);
	snprintf(line, sizeof(line), "%s %s", phrase1, phrase2);
	util_add_char(line, sizeof(line), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		DISPLAY_HEADER_HEIGHT / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line),
		DEFAULT_SCALE
	);
}

void UI::showAutoFooter()
{
	uint16_t halfSection = display_width() / 3 / 2;

	uint16_t x = 0;
	uint16_t y = DISPLAY_HEADER_HEIGHT + DISPLAY_CONTENT_HEIGHT + 1;
	uint16_t w = display_width() / 3;
	uint16_t h = display_height() - y;
	uint16_t curr_color = buttons[BTN_F1_Pin].pressed() ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	SENSOR_MODE mode = get_sensor_target_mode();
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
		strlen(line),
		DEFAULT_SCALE
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
		strlen(line),
		DEFAULT_SCALE
	);


	x += static_cast<uint16_t>(display_width() / 3);
	curr_color = buttons[BTN_F3_Pin].pressed() ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (mode == SENSOR_MODE_BIGSKI) {
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
		&bigski_bitmap,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line),
		DEFAULT_SCALE
	);
}

void UI::showManualFooter()
{
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
		strlen(linef2),
		DEFAULT_SCALE
	);

	x = 0;
	w = display_width() / 3;
	manual_f1_mode = SENSOR_MODE_SURFACE;
	sFONT* bitmap = &surface_bitmap;
	if (manual_f1_mode == get_sensor_mode() && !(get_sensor_target_mode() == SENSOR_MODE_BIGSKI)) {
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
		strlen(linef1),
		DEFAULT_SCALE
	);

	w -= 1;
	x = static_cast<uint16_t>(display_width() / 3 * 2 + 1);
	manual_f3_mode = SENSOR_MODE_STRING;
	bitmap = &string_bitmap;
	if (manual_f1_mode == manual_f3_mode || manual_f3_mode == get_sensor_mode()) {
		manual_f3_mode = SENSOR_MODE_BIGSKI;
		bitmap = &bigski_bitmap;
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
		strlen(linef3),
		DEFAULT_SCALE
	);
}

void UI::showServiceFooter()
{
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
		strlen(linef1),
		DEFAULT_SCALE
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
		strlen(linef3),
		DEFAULT_SCALE
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
			"%s: %d.%d",
			phrase,
			get_sensor_mode_target(get_sensor_mode()) / 100,
			__abs(get_sensor_mode_target(get_sensor_mode()) % 100) / 10
		);
		util_add_char(target, sizeof(target), ' ', (size_t)DISPLAY_WIDTH / u8g2_font_8x13_t_cyrillic.Width, ALIGN_MODE_CENTER);

		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			offset_x,
			offset_y,
			&u8g2_font_8x13_t_cyrillic,
			DISPLAY_ALIGN_CENTER,
			target,
			strlen(target),
			DEFAULT_SCALE
		);
	}

	{
		offset_y = display_height() / 2;
		char value[PHRASE_LEN_MAX] = {};
		const char* phrase = t(T_VALUE, settings.language);
		if (App::getRealValue() == App::SENSOR_VALUE_ERR) {
			snprintf(
				value,
				sizeof(value) - 1,
				"%s: %s",
				phrase,
				t(T_ERROR, settings.language)
			);
		} else {
			snprintf(
				value,
				sizeof(value) - 1,
				"%s: %d.%d",
				phrase,
				App::getRealValue() / 100,
				__abs(App::getRealValue() % 100) / 10
			);
		}
		util_add_char(value, sizeof(value), ' ', (size_t)DISPLAY_WIDTH / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			offset_x,
			offset_y,
			&u8g2_font_10x20_t_cyrillic,
			DISPLAY_ALIGN_CENTER,
			value,
			strlen(value),
			2
		);
	}
}

void UI::showLoading()
{
	char line[PHRASE_LEN_MAX] = {};
	snprintf(line, sizeof(line) - 1, "%s", t(loadStr, settings.language));
	util_add_char(line, sizeof(line), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		display_height() / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line),
		DEFAULT_SCALE
	);
}

void UI::showDirection(bool flag)
{
	static bool visible = true;
	static int8_t direction = (int8_t)0xFF;
	if (direction == get_sensor_direction() && visible == flag) {
		return;
	}
	direction = get_sensor_direction();
	visible = flag;

	uint16_t y = static_cast<uint16_t>(
		DISPLAY_HEADER_HEIGHT +
		DISPLAY_CONTENT_HEIGHT -
		DEFAULT_MARGIN -
		left_bitmap.Height
	);
	display_fill_rect(0, y, display_width(), left_bitmap.Height, DISPLAY_COLOR_WHITE);


	sFONT* font = &u8g2_font_10x20_t_cyrillic;
	char direction_line[TRANSLATE_MAX_LEN] = "";
	sFONT* bitmap = nullptr;
	uint16_t color = DISPLAY_COLOR_WHITE;
	switch (direction) {
	case STR_FORCE_LEFT:
		memcpy(
			direction_line,
			t(T_LEFT, settings.language),
			__min(strlen(t(T_LEFT, settings.language)), sizeof(direction_line))
		);
		bitmap = &left_bitmap;
		color = DISPLAY_COLOR_RED;
		break;
	case STR_LEFT:
		memcpy(
			direction_line,
			t(T_LEFT, settings.language),
			__min(strlen(t(T_LEFT, settings.language)), sizeof(direction_line))
		);
		bitmap = &left_bitmap;
		color = DISPLAY_COLOR_BLACK;
		break;
	case STR_FORCE_RIGHT:
		memcpy(
			direction_line,
			t(T_RIGHT, settings.language),
			__min(strlen(t(T_RIGHT, settings.language)), sizeof(direction_line))
		);
		bitmap = &right_bitmap;
		color = DISPLAY_COLOR_RED;
		break;
	case STR_RIGHT:
		memcpy(
			direction_line,
			t(T_RIGHT, settings.language),
			__min(strlen(t(T_RIGHT, settings.language)), sizeof(direction_line))
		);
		bitmap = &right_bitmap;
		color = DISPLAY_COLOR_BLACK;
		break;
	case STR_MIDDLE:
		bitmap = &left_bitmap;
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown STRING mode direction");
#endif
		fsm.push_event(error_e{});
		set_error(INTERNAL_ERROR);
		Error_Handler();
		return;
	};

	if (get_sensor_mode() != SENSOR_MODE_STRING || !visible) {
		color = DISPLAY_COLOR_WHITE;
	}

	uint16_t text_len = (uint16_t)(strlen(direction_line) * font->Width);
	uint16_t full_len = text_len + bitmap->Width;

	uint16_t x = 0;
	if (direction == STR_FORCE_LEFT || direction == STR_LEFT) {
		x = display_width() / 2 - full_len / 2;
	} else {
		x = (uint16_t)(display_width() / 2 + full_len / 2 - bitmap->Width);
	}
	char bitmap_line[] = " ";
	display_set_color(color);
	display_text_show(
		x,
		y,
		bitmap,
		DISPLAY_ALIGN_LEFT,
		bitmap_line,
		strlen(bitmap_line),
		DEFAULT_SCALE
	);


	if (direction == STR_FORCE_LEFT || direction == STR_LEFT) {
		x += bitmap->Width;
	} else {
		x -= text_len;
	}
	y += (uint16_t)(bitmap->Height / 2 - font->Height / 2);
	display_set_color(color);
	display_text_show(
		x,
		y,
		font,
		DISPLAY_ALIGN_LEFT,
		direction_line,
		strlen(direction_line),
		DEFAULT_SCALE
	);
}

void UI::showUp(bool flag)
{
	extern const BITMAPSTRUCT bmp_up_15x15;

	uint16_t x = DEFAULT_MARGIN;
	uint16_t y = static_cast<uint16_t>(
		DISPLAY_HEADER_HEIGHT +
		bmp_up_15x15.infoHeader.biHeight +
		u8g2_font_8x13_t_cyrillic.Height
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
	extern const BITMAPSTRUCT bmp_up_15x15;
	extern const BITMAPSTRUCT bmp_down_15x15;

	uint16_t x = DEFAULT_MARGIN;
	uint16_t y = static_cast<uint16_t>(
		DISPLAY_HEADER_HEIGHT +
		u8g2_font_8x13_t_cyrillic.Height +
		bmp_up_15x15.infoHeader.biHeight +
		DEFAULT_MARGIN +
		bmp_down_15x15.infoHeader.biHeight
	);

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
		strlen(line),
		DEFAULT_SCALE
	);

	fsm.push_event(success_e{});
}


void UI::_load_s::operator ()() const
{
	if (!is_status(WAIT_LOAD) &&
		!is_status(NEED_LOAD_SETTINGS) &&
		!is_status(NEED_SAVE_SETTINGS)
	) {
		fsm.push_event(success_e{});
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
	showMode();
	showHeader();
	showManualFooter();

	char line[PHRASE_LEN_MAX] = {};
	const char* phrase = t(T_NO_SENSOR, settings.language);
	snprintf(line, sizeof(line) - 1, "%s", phrase);
	uint32_t scale = 2;
	size_t width = display_width() / (u8g2_font_10x20_t_cyrillic.Width * scale);
	util_add_char(line, sizeof(line), ' ', width, ALIGN_MODE_CENTER);

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		display_height() / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		line,
		strlen(line),
		scale
	);

	if (!is_status(NO_SENSOR)) {
		fsm.push_event(sens_found_e{});
	}

	if (clicks.empty()) {
		return;
	}

	uint16_t click = clicks.pop_front();
	switch (click) {
	case BTN_F1_Pin:
		App::changeSensorMode(manual_f1_mode);
		fsm.push_event(sens_found_e{});
		break;
	case BTN_F2_Pin:
		fsm.push_event(service_e{});
		break;
	case BTN_F3_Pin:
		App::changeSensorMode(manual_f3_mode);
		fsm.push_event(sens_found_e{});
		break;
	case BTN_MODE_Pin:
	case BTN_ENTER_Pin:
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


void UI::_manual_mode_s::operator ()() const
{
	showMode();
	showManualFooter();
	showValue();
	showUp(is_status(MANUAL_NEED_VALVE_UP));
	showDown(is_status(MANUAL_NEED_VALVE_DOWN));
	showMiddle(__abs(App::getRealValue()) < App::getDeadBand());
	showDirection(get_sensor_mode() == SENSOR_MODE_STRING);

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
		reset_sensor_mode_target();
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
		if (App::getRealValue() == std::numeric_limits<int16_t>::max()) {
			break;
		}
		if (target_reseted) {
			target_reseted = false;
			break;
		}
		save_sensor_mode_target();
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
	showMiddle(__abs(App::getRealValue()) < App::getDeadBand());
	showDirection(get_sensor_mode() == SENSOR_MODE_STRING);

	if (App::getAppMode() == APP_MODE_MANUAL ||
		is_status(NO_SENSOR)
	) {
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
		App::changeSensorMode(SENSOR_MODE_BIGSKI);
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
	showServiceHeader();
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

	if (is_status(NEED_SERVICE_SAVE)) {
		loadStr = T_UPDATING_SETTINGS;
	}

	if (is_status(NEED_SERVICE_BACK)) {
		loadStr = T_RESETING_CHANGES;
	}

	if (is_status(NEED_SERVICE_SAVE) || is_status(NEED_SERVICE_BACK)) {
		reset_status(NEED_SERVICE_SAVE);
		reset_status(NEED_SERVICE_BACK);
		fsm.push_event(success_e{});
	}

	if (is_status(NEED_SERVICE_UPDATE)) {
		reset_status(NEED_SERVICE_UPDATE);
		serviceMenu->update();
	}

	serviceMenu->show();
}


void UI::_error_s::operator ()() const
{
	unsigned error = get_first_error();

	if (error) {
		uint16_t x = display_width() / 2;
		uint16_t y = display_height() / 2;

		char line[PHRASE_LEN_MAX] = {};
		const char* phrase = t(T_ERROR, settings.language);
		snprintf(line, sizeof(line) - 1, "%s %u", phrase, error);
		util_add_char(line, sizeof(line), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);
		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			x,
			y,
			&u8g2_font_10x20_t_cyrillic,
			DISPLAY_ALIGN_CENTER,
			line,
			strlen(line),
			DEFAULT_SCALE
		);

		y += (uint16_t)(u8g2_font_10x20_t_cyrillic.Height + DEFAULT_MARGIN);
		snprintf(line, sizeof(line) - 1, "%s", get_string_error((SOUL_STATUS)error, settings.language));
		util_add_char(line, sizeof(line), ' ', display_width() / u8g2_font_8x13_t_cyrillic.Width, ALIGN_MODE_CENTER);
		display_set_color(DISPLAY_COLOR_BLACK);
		display_set_color(DISPLAY_COLOR_BLACK);
		display_text_show(
			x,
			y,
			&u8g2_font_8x13_t_cyrillic,
			DISPLAY_ALIGN_CENTER,
			line,
			strlen(line),
			DEFAULT_SCALE
		);
	} else {
		loadStr = T_LOADING;
		fsm.push_event(success_e{});
	}
}

void UI::error_a::operator ()() const
{
	display_clear();

	showDown(false);
	showUp(false);
	showMiddle(false);
}

#define LOADING_DELAY_MS ((uint32_t)300)
void UI::load_start_a::operator ()() const
{
	fsm.clear_events();

	display_clear();
	timer.changeDelay(LOADING_DELAY_MS);

	showHeader();
}

void UI::no_sens_start_a::operator ()() const
{
	f1_color = DISPLAY_COLOR_WHITE;
	f2_color = DISPLAY_COLOR_WHITE;
	f3_color = DISPLAY_COLOR_WHITE;

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
	f1_color = DISPLAY_COLOR_WHITE;
	f2_color = DISPLAY_COLOR_WHITE;
	f3_color = DISPLAY_COLOR_WHITE;

	clicks.clear();
	fsm.clear_events();

	display_clear_header();
	display_clear_content();
	display_clear_footer();
	display_sections_show();


	char mode[PHRASE_LEN_MAX] = {};
	const char* phrase1 = t(T_MANUAL, settings.language);
	const char* phrase2 = t(T_MODE, settings.language);
	snprintf(mode, sizeof(mode), "%s %s", phrase1, phrase2);
	util_add_char(mode, sizeof(mode), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		DISPLAY_HEADER_HEIGHT / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		mode,
		strlen(mode),
		DEFAULT_SCALE
	);

	showHeader();
}

void UI::auto_start_a::operator ()() const
{
	f1_color = DISPLAY_COLOR_WHITE;
	f2_color = DISPLAY_COLOR_WHITE;
	f3_color = DISPLAY_COLOR_WHITE;

	clicks.clear();
	fsm.clear_events();

	display_clear_content();
	display_clear_footer();
	display_sections_show();

	char mode[PHRASE_LEN_MAX] = {};
	const char* phrase1 = t(T_AUTO, settings.language);
	const char* phrase2 = t(T_MODE, settings.language);
	snprintf(mode, sizeof(mode), "%s %s", phrase1, phrase2);
	util_add_char(mode, sizeof(mode), ' ', display_width() / u8g2_font_10x20_t_cyrillic.Width, ALIGN_MODE_CENTER);

	display_set_color(DISPLAY_COLOR_BLACK);
	display_text_show(
		display_width() / 2,
		DISPLAY_HEADER_HEIGHT / 2,
		&u8g2_font_10x20_t_cyrillic,
		DISPLAY_ALIGN_CENTER,
		mode,
		strlen(mode),
		DEFAULT_SCALE
	);

	showHeader();
}

void UI::service_start_a::operator ()() const
{
	f1_color = DISPLAY_COLOR_WHITE;
	f2_color = DISPLAY_COLOR_WHITE;
	f3_color = DISPLAY_COLOR_WHITE;

	clicks.clear();
	fsm.clear_events();

	display_clear_content();
	display_sections_show();

	serviceMenu->reset();
}
