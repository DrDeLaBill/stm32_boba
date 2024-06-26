/* Copyright © 2024 Georgy E. All rights reserved. */

#include "Menu.h"

#include <cstring>

#include <limits>

#include "main.h"
#include "soul.h"
#include "display.h"


#define IS_SPECIAL_BUTTON(button) ( \
								   button == BTN_F3_Pin ||  \
								   button == BTN_ENTER_Pin ||  \
								   button == BTN_F1_Pin \
								  )


Menu::Menu(uint16_t x, uint16_t y, uint16_t w, uint16_t h, MenuItem* items, uint16_t count):
	x(x), y(y + 1), w(w), h(h - 1), items(), count(count),
	start_idx(0), focused_idx(0), last_focused_idx(std::numeric_limits<uint16_t>::max()),
	real_start_idx(0), selected(false), needInit(true), timer(HOLD_TIMEOUT_MS),
	needUpdateSelected(false), needUpdateAll(false)
{
	this->items = std::make_unique<MenuItem[]>(count);
	for (uint16_t i = 0; i < count; i++) {
		this->items[i] = items[i];
		this->items[i].setWidth(w - SCROLL_WIDTH - 2);
	}
	for (uint16_t i = 0; i < count; i++) {
		if (this->items[i].isSelectable()) {
			real_start_idx = i;
			break;
		}
	}
	reset();
}

void Menu::reset()
{
	start_idx = 0;
	focused_idx = real_start_idx;
	last_focused_idx = std::numeric_limits<uint16_t>::max();
	needInit = true;
	selected = false;
}

void Menu::click(uint16_t button)
{
	if (selected && !IS_SPECIAL_BUTTON(button)) {
		items[focused_idx].click(button);
		needUpdateSelected = true;
		return;
	}

	switch (button) {
	case BTN_ENTER_Pin:
		selected = !selected;
		needUpdateSelected = true;
		items[focused_idx].setSelected(selected);
		break;
	case BTN_F3_Pin:
		items[focused_idx].setSelected(false);
		set_status(NEED_SAVE_SETTINGS);
		set_status(NEED_SERVICE_SAVE);
		break;
	case BTN_UP_Pin:
		do {
			focused_idx = focused_idx > 0 ? focused_idx - 1 : count - 1;
		} while (!items[focused_idx].isSelectable() && focused_idx != 0);
		if (!items[focused_idx].isSelectable()) {
			focused_idx = count - 1;
		}
		break;
	case BTN_DOWN_Pin:
		do {
			focused_idx = focused_idx < count - 1 ? focused_idx + 1 : 0;
		} while (!items[focused_idx].isSelectable() && focused_idx != count - 1);
		if (!items[focused_idx].isSelectable()) {
			focused_idx = 0;
		}
		break;
	case BTN_F1_Pin:
		if (selected) {
			needUpdateSelected = true;
			selected = false;
		} else {
			set_status(NEED_SERVICE_BACK);
			set_status(NEED_LOAD_SETTINGS);
		}
		items[focused_idx].setSelected(selected);
		break;
	case BTN_MODE_Pin:
	case BTN_F2_Pin:
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

void Menu::holdUp()
{
	if (timer.wait()) {
		return;
	}

	click(BTN_UP_Pin);
	timer.start();
}

void Menu::holdDown()
{
	if (timer.wait()) {
		return;
	}

	click(BTN_DOWN_Pin);
	timer.start();
}

void Menu::update()
{
	needUpdateAll = true;
}

void Menu::show()
{
	if (needUpdateSelected && focused_idx == last_focused_idx) {
		needUpdateSelected = false;
		items[focused_idx].setNeedUpdate(true);
		items[focused_idx].show();
		items[focused_idx].setNeedUpdate(false);
	}

	if (!needUpdateAll && focused_idx == last_focused_idx) {
		return;
	}

	while (!items[focused_idx].isSelectable() && focused_idx != count - 1) {
		focused_idx = focused_idx < count - 1 ? focused_idx + 1 : 0;
	}

	uint16_t focused_height = 0;
	uint16_t tmp_start_idx = 0;
	for (uint16_t i = focused_idx + 1; i > 0; i--) {
		focused_height += items[i - 1].height();
		if (focused_height > h) {
			break;
		}
		tmp_start_idx = i - 1;
	}

	bool need_scroll = false;
	if (start_idx > focused_idx) {
		start_idx = focused_idx;
		need_scroll = true;
	}
	if (start_idx < tmp_start_idx) {
		start_idx = tmp_start_idx;
		need_scroll = true;
	}
	if (focused_idx == real_start_idx) {
		start_idx = 0;
		need_scroll = true;
	}
	if (needInit || needUpdateAll) {
		need_scroll = true;
	}

	if (!need_scroll) {
		items[last_focused_idx].setNeedUpdate(true);
		items[last_focused_idx].setFocused(false);
		items[last_focused_idx].show();
		items[last_focused_idx].setNeedUpdate(false);

		items[focused_idx].setNeedUpdate(true);
		items[focused_idx].setFocused(true);
		items[focused_idx].show();
		items[focused_idx].setNeedUpdate(false);
	}

	uint16_t curr_height = 0;
	for (unsigned i = start_idx; i < count; i++) {
		if (!need_scroll) {
			break;
		}
		if (curr_height + items[i].height() >= h) {
			break;
		}
		items[i].setNeedUpdate(need_scroll);
		items[i].setY((uint16_t)(y + curr_height));
		items[i].setFocused(focused_idx == i);
		items[i].show();
		items[i].setNeedUpdate(false);
		curr_height += items[i].height();
	}

	if (needInit) {
		uint16_t scroll_x = (uint16_t)(x + w - SCROLL_WIDTH);
		display_fill_rect(scroll_x, y, SCROLL_WIDTH, h, DISPLAY_COLOR_LIGHT_GRAY);
		needInit = false;
	} else if (last_focused_idx != focused_idx) {
		uint16_t new_x = (uint16_t)(x + w - SCROLL_WIDTH);
		uint16_t old_y = (uint16_t)util_convert_range(last_focused_idx, real_start_idx, count - 1, y, y + h - SCROLL_HEIGHT);
		display_fill_rect(new_x, old_y, SCROLL_WIDTH, SCROLL_HEIGHT, DISPLAY_COLOR_LIGHT_GRAY);
	}

	if (last_focused_idx != focused_idx) {
		uint16_t new_x = (uint16_t)(x + w - SCROLL_WIDTH);
		uint16_t new_y = (uint16_t)util_convert_range(focused_idx, real_start_idx, count - 1, y, y + h - SCROLL_HEIGHT);
		display_fill_rect(new_x, new_y, SCROLL_WIDTH, SCROLL_HEIGHT, DISPLAY_COLOR_LIGHT_GRAY2);
		last_focused_idx = focused_idx;
	}

	needUpdateAll = false;
}

unsigned Menu::itemsCount()
{
	return count;
}
