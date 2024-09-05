/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _UI_H_
#define _UI_H_


#include <memory>
#include <utility>
#include <cstdint>
#include <unordered_map>


#define UI_BEDUG (false)


struct UI
{
private:
	static constexpr uint32_t DEBOUNCE_MS = 20;

public:
	static constexpr char TAG[] = "UI";

	static void showUp(bool flag = false);
	static void showDown(bool flag = false);
	static void showMiddle(bool flag = false);

	UI();

	void tick();

	void buttonsTick();

};


#endif
