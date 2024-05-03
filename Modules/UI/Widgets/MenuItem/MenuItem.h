/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _MENU_ITEM_H_
#define _MENU_ITEM_H_


#include <cstdint>
#include <cstring>

#include "utils.h"
#include "display.h"


struct IMenuCallback
{
	virtual void click(uint16_t) = 0;
	virtual char* value() = 0;
};


struct MenuItem
{
public:
	static const unsigned LABEL_MAX_LEN = 40;
	static const unsigned VALUE_MAX_LEN = 20;

private:
	static const unsigned MARGIN = 3;

	uint16_t x;
	uint16_t y;
	uint16_t w;
	sFONT* font;

	bool focused;
	bool selected;

	char label[LABEL_MAX_LEN];
	char value[VALUE_MAX_LEN];

	uint16_t background;

	IMenuCallback* callback;

	bool selectable;
	bool needUpdate;

public:

	MenuItem();
	MenuItem(const MenuItem& other);
	MenuItem(IMenuCallback* callback, const bool selectable = true, const char* label = "", const char* value = "", sFONT* font = &Font12);
	~MenuItem();

	MenuItem& operator=(const MenuItem& other);

	void setX(const uint16_t x);
	void setY(const uint16_t y);
	void setWidth(const uint16_t w);
	void setFont(sFONT* font);
	void setLabel(const char* label, const unsigned length);
	void setFocused(bool focused);
	void setSelected(bool selected);
	void setCallback(IMenuCallback& callback);
	void setNeedUpdate(bool state);

	bool isFocused();
	bool isSelectable();

	uint16_t getX();
	uint16_t getY();
	uint16_t height();

	void show();

	void hide();

	void click(uint16_t button);

};


#endif
