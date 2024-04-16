/* Copyright © 2024 Georgy E. All rights reserved. */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "bmp.h"
#include "fonts.h"
#include "ili9341.h"

#include "stm32_adafruit_lcd.h"


#define DISPLAY_HEIGHT           ((uint16_t)ILI9341_LCD_PIXEL_HEIGHT)
#define DISPLAY_WIDTH            ((uint16_t)ILI9341_LCD_PIXEL_WIDTH)

#define DISPLAY_HEADER_HEIGHT    (DISPLAY_HEIGHT / 6)
#define DISPLAY_FOOTER_HEIGHT    (DISPLAY_HEIGHT / 6)
#define DISPLAY_CONTENT_HEIGHT   (DISPLAY_HEIGHT - DISPLAY_HEADER_HEIGHT - DISPLAY_FOOTER_HEIGHT)


#define __COLOR(color)           (RC(((uint16_t)(color) ^ 0xFFFF)))

#define DISPLAY_COLOR_RED        __COLOR(0xF800)
#define DISPLAY_COLOR_GREEN      __COLOR(0x07E0)
#define DISPLAY_COLOR_BLUE       __COLOR(0x001F)
#define DISPLAY_COLOR_WHITE      __COLOR(LCD_COLOR_WHITE)
#define DISPLAY_COLOR_BLACK      __COLOR(LCD_COLOR_BLACK)

#define DISPLAY_DEFAULT_COLOR    DISPLAY_COLOR_WHITE


typedef enum _DISPLAY_ALIGN_MODE
{
	DISPLAY_ALIGN_CENTER = 1,
	DISPLAY_ALIGN_LEFT,
	DISPLAY_ALIGN_RIGHT
} DISPLAY_ALIGN_MODE;


void display_init();

uint16_t display_height();
uint16_t display_width();

void display_clear();
void display_clear_header();
void display_clear_content();
void display_clear_footer();
void display_clear_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void display_sections_show();
void display_set_color(uint16_t color);
void display_text_show(
	const uint16_t x,
	const uint16_t y,
	sFONT* font,
	DISPLAY_ALIGN_MODE mode,
	const char* text,
	const unsigned len
);

void display_draw_bitmap(uint16_t x, uint16_t y, const BITMAPSTRUCT* bmp);


#ifdef __cplusplus
}
#endif


#endif
