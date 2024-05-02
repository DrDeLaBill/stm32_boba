/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Menu.h"

#include <cstring>

#include "main.h"
#include "soul.h"
#include "display.h"

Menu::Menu(MenuItem* items, unsigned size):
	items(), count(size), focused_idx(0), selected(false)
{
	this->items = std::make_unique<MenuItem[]>(size);
	for (uint16_t i = 0; i < size; i++) {
		this->items[i] = items[i];
		this->items[i].setY(DISPLAY_HEADER_HEIGHT + MARGIN + i * this->items[i].height());
	}
}

void Menu::click(uint16_t button)
{
	if (selected) {
		items[focused_idx].click(button);
		return;
	}

	switch (button) {
	case BTN_ENTER_Pin:
		selected = true;
		break;
	case BTN_UP_Pin:
		focused_idx = focused_idx > 0 ? focused_idx - 1 : count - 1;
		break;
	case BTN_DOWN_Pin:
		focused_idx = focused_idx < count - 1 ? focused_idx + 1 : 0;
		break;
	case BTN_MODE_Pin:
	case BTN_F1_Pin:
	case BTN_F2_Pin:
	case BTN_F3_Pin:
		break;
	default:
#ifdef DEBUG
		BEDUG_ASSERT(false, "Unknown menu handler");
#endif
		set_error(INTERNAL_ERROR);
		Error_Handler();
		break;
	}
}

void Menu::show()
{
	for (unsigned i = 0; i < count; i++) {
		items[i].setSelected(focused_idx == i);
		items[i].show();
	}
}

void Menu::setValue(const unsigned index, const char* value)
{
	if (index >= count) {
		BEDUG_ASSERT(false, "Menu items is out of range");
		return;
	}
	items[index].setValue(value, strlen(value));
}

unsigned Menu::itemsCount()
{
	return count;
}
