/* Copyright © 2024 Georgy E. All rights reserved. */

#include "MenuItem.h"

#include "gstring.h"


MenuItem::MenuItem():
	x(0), y(0), w(0), font(&Font12), focused(false), selected(false),
	background(DISPLAY_COLOR_BLACK), selectable(true), needUpdate(false)
{}

MenuItem::MenuItem(const MenuItem& other)
{
	*this = other;
}

MenuItem::MenuItem(IMenuCallback* callback, const bool selectable, sFONT* font):
	x(0), y(0), w(0), font(font), focused(false), selected(false),
	background(DISPLAY_COLOR_BLACK), callback(callback), selectable(selectable), needUpdate(false)
{}

MenuItem::~MenuItem()
{}

MenuItem& MenuItem::operator=(const MenuItem& other)
{
	x = other.x;
	y = other.y;
	font = other.font;
	focused = other.focused;
	background = other.background;
	callback = other.callback;
	selectable = other.selectable;
	return *this;
}

void MenuItem::setX(const uint16_t x)
{
	this->x = x;
}

void MenuItem::setY(const uint16_t y)
{
	this->y = y;
}

void MenuItem::setWidth(const uint16_t w)
{
	this->w = w;
}

void MenuItem::setFont(sFONT* font)
{
	this->font = font;
}

void MenuItem::setFocused(bool focused)
{
	this->focused = focused;
}

void MenuItem::setSelected(bool selected)
{
	this->selected = selected;
}

void MenuItem::setCallback(IMenuCallback& callback)
{
	this->callback = &callback;
}

void MenuItem::setNeedUpdate(bool state)
{
	needUpdate = state;
}

bool MenuItem::isFocused()
{
	return focused;
}

bool MenuItem::isSelectable()
{
	return selectable;
}

uint16_t MenuItem::getX()
{
	return x;
}

uint16_t MenuItem::getY()
{
	return y;
}

uint16_t MenuItem::height()
{
	return font->Height + 2 * MARGIN;
}

void MenuItem::show()
{
	unsigned value_index = (w / 3 * 2) / font->Width;
	char line[LABEL_MAX_LEN + VALUE_MAX_LEN + 1] = "";
	memset(line, ' ', sizeof(line));
	line[sizeof(line) - 1] = 0;

	memcpy(line, callback->label(), strlen(callback->label()));

	memcpy(&line[value_index], callback->value(), strlen(callback->value()));
	line[value_index + strlen(callback->value())] = 0;
	util_add_char(
		line,
		sizeof(line),
		' ',
		w / font->Width,
		(selectable || strlen(callback->value())) ? ALIGN_MODE_LEFT : ALIGN_MODE_CENTER
	);

	uint16_t curr_color = DISPLAY_COLOR_WHITE;
	if (selected) {
		curr_color = DISPLAY_COLOR_LIGHT_GRAY2;
	} else if (focused) {
		curr_color = DISPLAY_COLOR_LIGHT_GRAY;
	}
	if (needUpdate || background != curr_color) {
		background = curr_color;
		display_fill_rect(x, y, w, height(), background);
	}

	uint16_t max_len = (uint16_t)((w - 2 * MARGIN) / font->Width);
	display_set_color(DISPLAY_COLOR_BLACK);
	display_set_background(background);
	display_text_show(x + MARGIN, y + MARGIN, font, DISPLAY_ALIGN_LEFT, line, __min(max_len, strlen(line)), 1);
}

void MenuItem::hide()
{
	display_clear_rect(x, y, w, font->Height + 2 * MARGIN);
}

void MenuItem::click(uint16_t button)
{
	callback->click(button);
}
