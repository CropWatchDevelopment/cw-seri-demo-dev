#pragma once

#include <stdbool.h>
#include "stm32l0xx_hal.h"

#define SSD1306_WIDTH  128U
#define SSD1306_HEIGHT 64U

void ssd1306_init(I2C_HandleTypeDef* hi2c);
void ssd1306_clear(void);
void ssd1306_fill(uint8_t value);
void ssd1306_update(void);
void ssd1306_draw_string(uint8_t x, uint8_t y, const char* text);
void ssd1306_draw_string_scaled(uint8_t x, uint8_t y, const char* text, uint8_t scale);
void ssd1306_draw_bitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t* bitmap);
uint8_t ssd1306_measure_text_width(const char* text, uint8_t scale);
bool ssd1306_is_initialized(void);
#define SSD1306_FONT_WIDTH   5U
#define SSD1306_FONT_HEIGHT  7U
#define SSD1306_FONT_SPACING 1U
