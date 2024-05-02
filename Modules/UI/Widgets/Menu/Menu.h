/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _MENU_H_
#define _MENU_H_


#include <memory>

#include "MenuItem.h"


class Menu
{
private:
	static const uint16_t MARGIN = 10;

	std::unique_ptr<MenuItem[]> items;
	unsigned count;

	unsigned focused_idx;
	bool selected;

public:
	Menu(MenuItem* items, unsigned size);

	void click(uint16_t button);

	void show();

	void setValue(const unsigned index, const char* value);

	unsigned itemsCount();

};


#endif
