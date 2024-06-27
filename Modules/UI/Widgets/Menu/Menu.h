/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _MENU_H_
#define _MENU_H_


#include <memory>
#include <cstdint>

#include "Timer.h"
#include "MenuItem.h"


class Menu
{
private:
	static const uint16_t MARGIN = 10;

	static const uint16_t SCROLL_HEIGHT = 40;
	static const uint16_t SCROLL_WIDTH = 10;

	static const uint32_t HOLD_TIMEOUT_MS = 100;

	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;

	std::unique_ptr<MenuItem[]> items;
	uint16_t count;

	uint16_t start_idx;
	uint16_t focused_idx;
	uint16_t last_focused_idx;
	uint16_t real_start_idx;
	bool selected;
	bool needInit;

	utl::Timer timer;

	bool needUpdateSelected;
	bool needUpdateAll;

public:
	Menu(uint16_t x, uint16_t y, uint16_t w, uint16_t h, MenuItem* items, uint16_t size);

	void reset();

	void click(uint16_t button);
	void holdUp();
	void holdDown();
	void update();

	void show();

	unsigned itemsCount();

};


#endif
