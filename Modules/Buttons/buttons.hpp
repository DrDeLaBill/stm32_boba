/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _BUTTONS_HPP_
#define _BUTTONS_HPP_


#include <cstdint>

#include "CircleBuffer.h"


#define UI_CLICKS_SIZE (8)


extern utl::circle_buffer<UI_CLICKS_SIZE, uint16_t> clicks;

void ui_btn_tick();


#endif
