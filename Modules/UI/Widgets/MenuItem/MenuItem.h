/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _MENU_ITEM_H_
#define _MENU_ITEM_H_


#include <cstdint>

#include "display.h"


class MenuItem
{
public:
	static const unsigned LABEL_MAX_LEN = 40;
	static const unsigned VALUE_MAX_LEN = 20;

private:
	static const unsigned MARGIN = 3;

	uint16_t x;
	uint16_t y;
	sFONT* font;

	bool selected;

	char label[LABEL_MAX_LEN];
	char value[VALUE_MAX_LEN];

	uint16_t color;

public:
	MenuItem();
	MenuItem(const MenuItem& other);
	MenuItem(const char* label, const char* value, sFONT* font = &Font12);
	~MenuItem();

	MenuItem& operator=(const MenuItem& other);

	void setX(const uint16_t x);
	void setY(const uint16_t y);
	void setFont(sFONT* font);
	void setLabel(const char* label, const unsigned length);
	void setValue(const char* value, const unsigned length);
	void setSelected(bool selected);

	bool isSelected();

	uint16_t height();

	void show();
	void hide();

	void click(uint16_t button);

};


#endif
