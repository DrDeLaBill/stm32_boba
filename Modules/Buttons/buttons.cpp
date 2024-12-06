/* Copyright © 2024 Georgy E. All rights reserved. */

#include "buttons.hpp"

#include <unordered_map>

#include "main.h"

#include "Button.h"
#include "CodeStopwatch.h"


utl::circle_buffer<UI_CLICKS_SIZE, uint16_t> clicks;
static std::unordered_map<uint16_t, Button> buttons = {
	{BTN_F1_Pin,    {BTN_F1_GPIO_Port,    BTN_F1_Pin,    true}},
	{BTN_DOWN_Pin,  {BTN_DOWN_GPIO_Port,  BTN_DOWN_Pin,  true}},
	{BTN_UP_Pin,    {BTN_UP_GPIO_Port,    BTN_UP_Pin,    true}},
	{BTN_ENTER_Pin, {BTN_ENTER_GPIO_Port, BTN_ENTER_Pin, true}},
	{BTN_MODE_Pin,  {BTN_MODE_GPIO_Port,  BTN_MODE_Pin,  true}},
	{BTN_F2_Pin,    {BTN_F2_GPIO_Port,    BTN_F2_Pin,    true}},
	{BTN_F3_Pin,    {BTN_F3_GPIO_Port,    BTN_F3_Pin,    true}}
};


void ui_btn_tick()
{
	utl::CodeStopwatch watch("UI1", 100);

	for (auto& button : buttons) {
		button.second.tick();
		if (button.second.oneClick()) {
			clicks.push_back(button.first);
		}
	}
}
