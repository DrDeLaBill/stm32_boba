/* Copyright © 2024 Georgy E. All rights reserved. */

#include "MenuItem.h"

#include <cstring>

#include "utils.h"


MenuItem::MenuItem():
	x(0), y(0), font(&Font16), selected(false), label(""), value(""), color(DISPLAY_COLOR_BLACK)
{}

MenuItem::MenuItem(const MenuItem& other)
{
	*this = other;
}

MenuItem::MenuItem(const char* label, const char* value, sFONT* font):
	x(0), y(0), font(font), selected(false), label(""), value(""), color(DISPLAY_COLOR_BLACK)
{
	memcpy(this->label, label, __min(strlen(label), sizeof(this->label)));
	memcpy(this->value, value, __min(strlen(value), sizeof(this->value)));
}

MenuItem::~MenuItem()
{}

MenuItem& MenuItem::operator=(const MenuItem& other)
{
	x = other.x;
	y = other.y;
	font = other.font;
	selected = other.selected;
	memcpy(label, other.label, sizeof(label));
	memcpy(value, other.value, sizeof(value));
	color = other.color;
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

void MenuItem::setFont(sFONT* font)
{
	this->font = font;
}

void MenuItem::setLabel(const char* label, const unsigned length)
{
	memcpy(this->label, label, __min(length, sizeof(this->label) - 1));
}

void MenuItem::setValue(const char* value, const unsigned length)
{
	memcpy(this->value, value, __min(length, sizeof(this->value) - 1));
}

void MenuItem::setSelected(bool selected)
{
	this->selected = selected;
}

bool MenuItem::isSelected()
{
	return selected;
}

uint16_t MenuItem::height()
{
	return font->Height + 2 * MARGIN;
}

void MenuItem::show()
{
	unsigned value_index = (display_width() / 4 * 3) / font->Width;
	char line[LABEL_MAX_LEN + VALUE_MAX_LEN + 1] = "";
	memset(line, ' ', sizeof(line));
	line[sizeof(line) - 1] = 0;

	memcpy(line, label, strlen(label));
	memcpy(&line[value_index], value, strlen(value));
	line[value_index + strlen(value)] = 0;

	uint16_t curr_color = selected ? DISPLAY_COLOR_LIGHT_GRAY : DISPLAY_COLOR_WHITE;
	if (color != curr_color) {
		color = curr_color;
		display_fill_rect(x, y, display_width(), height(), color);
	}

	display_set_color(DISPLAY_COLOR_BLACK);
	display_set_background(color);
	display_text_show(x + MARGIN, y + MARGIN, font, DISPLAY_ALIGN_LEFT, line, strlen(line));
}

void MenuItem::hide()
{
	display_clear_rect(x, y, display_width(), font->Height + 2 * MARGIN);
}

void MenuItem::click(uint16_t button)
{

}
