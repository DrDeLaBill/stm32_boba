/* Copyright © 2024 Georgy E. All rights reserved. */

#include "display.h"

#include <stdint.h>

#include "cmsis_gcc.h"

#include "bmacro.h"
#include "hal_defs.h"
#include "stm32_adafruit_lcd.h"



void display_init()
{
	BSP_LCD_Init();
	BSP_LCD_Clear(DISPLAY_COLOR_WHITE);
}

uint16_t display_height()
{
	return DISPLAY_HEIGHT;
}

uint16_t display_width()
{
	return DISPLAY_WIDTH;
}

void display_clear()
{
	BSP_LCD_Clear(DISPLAY_COLOR_WHITE);
}

void display_clear_header()
{
    display_clear_rect(0, 0, DISPLAY_WIDTH, DISPLAY_HEADER_HEIGHT - 1);
}

void display_clear_content()
{
    display_clear_rect(0, DISPLAY_HEADER_HEIGHT + 1, DISPLAY_WIDTH, DISPLAY_CONTENT_HEIGHT - 1);
}

void display_clear_footer()
{
    display_clear_rect(0, DISPLAY_HEADER_HEIGHT + DISPLAY_CONTENT_HEIGHT + 1, DISPLAY_WIDTH, DISPLAY_FOOTER_HEIGHT);
}

void display_clear_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	display_fill_rect(x, y, w, h, DISPLAY_COLOR_WHITE);
}

void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    BSP_LCD_SetTextColor(color);
    BSP_LCD_FillRect(x, y, w, h);
}

void display_sections_show()
{
    BSP_LCD_SetTextColor(DISPLAY_COLOR_BLACK);

    uint16_t y = (uint16_t)DISPLAY_HEADER_HEIGHT;
    BSP_LCD_DrawLine(0, y, DISPLAY_WIDTH, y);

    y = DISPLAY_HEADER_HEIGHT + DISPLAY_CONTENT_HEIGHT;
    BSP_LCD_DrawLine(0, y, DISPLAY_WIDTH, y);

    uint16_t x = DISPLAY_WIDTH / 3;
    BSP_LCD_DrawLine(x, y, x, DISPLAY_HEIGHT);

    x = 2 * DISPLAY_WIDTH / 3;
	BSP_LCD_DrawLine(x, y, x, DISPLAY_HEIGHT);
}

void display_set_background(uint16_t color)
{
    BSP_LCD_SetBackColor(color);
}

void display_set_color(uint16_t color)
{
    BSP_LCD_SetTextColor(color);
}

void display_text_show(
	const uint16_t x,
	const uint16_t y,
	sFONT* font,
	DISPLAY_ALIGN_MODE mode,
	const char* text,
	const unsigned len,
	const uint32_t scale
) {
    BSP_LCD_SetFont(font);

    uint16_t tmp_x = x, tmp_y = y - font->Height * scale;

    switch (mode) {
    case DISPLAY_ALIGN_CENTER:
    	tmp_x -= (uint16_t)((len * font->Width * scale) / 2);
    	tmp_y -= (uint16_t)(font->Height * scale / 2);
    	break;
    case DISPLAY_ALIGN_LEFT:
    	break;
    case DISPLAY_ALIGN_RIGHT:
    	tmp_x -= (uint16_t)(len * font->Width * scale);
    	break;
    default:
    	break;
    }

    for (unsigned i = 0; i < len; i++) {
        BSP_LCD_DisplayChar(
			(uint16_t)(tmp_x + i * font->Width * scale),
			(uint16_t)(tmp_y + font->Height * scale),
			(uint8_t)text[i],
			scale
        );
    }

    BSP_LCD_SetBackColor(DISPLAY_COLOR_WHITE);
    BSP_LCD_SetTextColor(DISPLAY_DEFAULT_COLOR);
}

void display_draw_bitmap(uint16_t x, uint16_t y, const BITMAPSTRUCT* bmp)
{
	BEDUG_ASSERT(x + bmp->infoHeader.biWidth <= DISPLAY_WIDTH, "Bitmap x position is out of display size");
	BEDUG_ASSERT(y + bmp->infoHeader.biHeight <= DISPLAY_HEIGHT, "Bitmap y position is out of display size");
	BSP_LCD_DrawBitmap(x, y, (uint8_t *)bmp);
}
