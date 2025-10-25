#include "ssd1306.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define SSD1306_I2C_ADDRESS (0x3CU << 1)  // 0x78 on the wire
#define SSD1306_PAGE_COUNT  (SSD1306_HEIGHT / 8U)
#define GLYPH_WIDTH         SSD1306_FONT_WIDTH
#define GLYPH_HEIGHT        7U

typedef struct {
    char character;
    uint8_t rows[GLYPH_HEIGHT];
} glyph_t;

static const glyph_t* find_glyph(char c);
static void ssd1306_set_pixel(uint8_t x, uint8_t y, bool on);
static void ssd1306_draw_char_scaled(uint8_t x, uint8_t y, char c, uint8_t scale);
static void ssd1306_send_command(uint8_t cmd);
static void ssd1306_send_data(const uint8_t* data, size_t length);

static I2C_HandleTypeDef* ssd1306_i2c = NULL;
static uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_PAGE_COUNT];

static const glyph_t font_table[] = {
    {' ', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {'-', {0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00}},
    {'.', {0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x00}},
    {':', {0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00}},
    {'%', {0x19, 0x1A, 0x04, 0x08, 0x10, 0x13, 0x13}},
    {'0', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'1', {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {'2', {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}},
    {'3', {0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E}},
    {'4', {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}},
    {'5', {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}},
    {'6', {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}},
    {'7', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}},
    {'8', {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}},
    {'9', {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}},
    {'A', {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'B', {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}},
    {'C', {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}},
    {'D', {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}},
    {'E', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}},
    {'F', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}},
    {'G', {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E}},
    {'H', {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'I', {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {'J', {0x1F, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0C}},
    {'K', {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}},
    {'L', {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}},
    {'M', {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}},
    {'N', {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}},
    {'O', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'P', {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}},
    {'Q', {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}},
    {'R', {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
    {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
    {'T', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
    {'U', {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'V', {0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04}},
    {'W', {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}},
    {'X', {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}},
    {'Y', {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}},
    {'Z', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}},
    {'?', {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04}},
};

void ssd1306_init(I2C_HandleTypeDef* hi2c) {
    if (hi2c == NULL) {
        return;
    }

    if (HAL_I2C_IsDeviceReady(hi2c, SSD1306_I2C_ADDRESS, 2, 100) != HAL_OK) {
        return;
    }

    ssd1306_i2c = hi2c;
    HAL_Delay(100);

    static const uint8_t init_sequence[] = {
        0xAE,       // display off
        0x20, 0x00, // horizontal addressing mode
        0xB0,       // page start address
        0xC8,       // COM scan direction remapped (panel mounted upside-down)
        0x00,       // low column address
        0x10,       // high column address
        0x40,       // start line address
        0x81, 0x7F, // contrast control
        0xA1,       // column address 127 is mapped to SEG0
        0xA6,       // normal display
        0xA8, 0x3F, // multiplex ratio
        0xA4,       // resume to RAM content display
        0xD3, 0x00, // display offset
        0xD5, 0x80, // display clock divide ratio/oscillator frequency
        0xD9, 0xF1, // pre-charge period
        0xDA, 0x12, // COM pins hardware configuration
        0xDB, 0x40, // VCOMH deselect level
        0x8D, 0x14, // charge pump
        0xAF        // display on
    };

    for (size_t i = 0; i < sizeof(init_sequence); i++) {
        ssd1306_send_command(init_sequence[i]);
    }

    ssd1306_clear();
    ssd1306_update();
}

void ssd1306_clear(void) {
    ssd1306_fill(0x00);
}

void ssd1306_fill(uint8_t value) {
    memset(ssd1306_buffer, value, sizeof(ssd1306_buffer));
}

bool ssd1306_is_initialized(void) {
    return (ssd1306_i2c != NULL);
}

void ssd1306_update(void) {
    if (!ssd1306_is_initialized()) {
        return;
    }

    for (uint8_t page = 0; page < SSD1306_PAGE_COUNT; page++) {
        ssd1306_send_command(0xB0U + page);
        ssd1306_send_command(0x00U);
        ssd1306_send_command(0x10U);

        uint16_t offset = page * SSD1306_WIDTH;
        const uint8_t* page_data = &ssd1306_buffer[offset];
        size_t remaining = SSD1306_WIDTH;

        while (remaining > 0U) {
            size_t chunk = (remaining > 16U) ? 16U : remaining;
            ssd1306_send_data(page_data, chunk);
            page_data += chunk;
            remaining -= chunk;
        }
    }
}

void ssd1306_draw_string(uint8_t x, uint8_t y, const char* text) {
    ssd1306_draw_string_scaled(x, y, text, 1U);
}

void ssd1306_draw_string_scaled(uint8_t x, uint8_t y, const char* text, uint8_t scale) {
    if (text == NULL || scale == 0U) {
        return;
    }

    uint8_t cursor_x = x;
    uint8_t cursor_y = y;
    uint8_t scaled_height = (uint8_t)(GLYPH_HEIGHT * scale);
    uint8_t scaled_width = (uint8_t)(GLYPH_WIDTH * scale);
    uint8_t scaled_spacing = (uint8_t)(SSD1306_FONT_SPACING * scale);

    for (size_t idx = 0; text[idx] != '\0'; idx++) {
        char c = text[idx];
        if (c == '\n') {
            cursor_y = (uint8_t)(cursor_y + scaled_height + scaled_spacing);
            cursor_x = x;
            continue;
        }

        if ((cursor_x + scaled_width) > SSD1306_WIDTH ||
            (cursor_y + scaled_height) > SSD1306_HEIGHT) {
            break;
        }

        ssd1306_draw_char_scaled(cursor_x, cursor_y, c, scale);
        cursor_x = (uint8_t)(cursor_x + scaled_width + scaled_spacing);
    }
}

uint8_t ssd1306_measure_text_width(const char* text, uint8_t scale) {
    if (text == NULL || scale == 0U) {
        return 0U;
    }

    size_t count = 0;
    for (size_t idx = 0; text[idx] != '\0'; idx++) {
        if (text[idx] == '\n') {
            break;
        }
        count++;
    }

    if (count == 0U) {
        return 0U;
    }

    uint16_t glyph_width = (uint16_t)(GLYPH_WIDTH * scale);
    uint16_t spacing = (uint16_t)(SSD1306_FONT_SPACING * scale);
    uint16_t width = (uint16_t)(count * glyph_width);
    width += (uint16_t)((count - 1U) * spacing);
    if (width > 255U) {
        width = 255U;
    }
    return (uint8_t)width;
}

static void ssd1306_draw_char_scaled(uint8_t x, uint8_t y, char c, uint8_t scale) {
    const glyph_t* glyph = find_glyph(c);
    if (glyph == NULL) {
        glyph = find_glyph('?');
    }

    for (uint8_t row = 0; row < GLYPH_HEIGHT; row++) {
        uint8_t row_bits = glyph->rows[row];
        for (uint8_t col = 0; col < GLYPH_WIDTH; col++) {
            uint8_t bit_index = (uint8_t)(GLYPH_WIDTH - 1U - col);
            bool pixel_on = ((row_bits >> bit_index) & 0x01U) != 0U;
            if (!pixel_on) {
                continue;
            }
            for (uint8_t dy = 0; dy < scale; dy++) {
                for (uint8_t dx = 0; dx < scale; dx++) {
                    ssd1306_set_pixel((uint8_t)(x + col * scale + dx),
                                      (uint8_t)(y + row * scale + dy),
                                      true);
                }
            }
        }
    }
}

static void ssd1306_set_pixel(uint8_t x, uint8_t y, bool on) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }

    uint16_t index = (uint16_t)(x + (y / 8U) * SSD1306_WIDTH);
    uint8_t bit_mask = (uint8_t)(1U << (y & 0x07U));

    if (on) {
        ssd1306_buffer[index] |= bit_mask;
    } else {
        ssd1306_buffer[index] &= (uint8_t)~bit_mask;
    }
}

static const glyph_t* find_glyph(char c) {
    size_t count = sizeof(font_table) / sizeof(font_table[0]);
    for (size_t i = 0; i < count; i++) {
        if (font_table[i].character == c) {
            return &font_table[i];
        }
    }
    return NULL;
}

static void ssd1306_send_command(uint8_t cmd) {
    if (!ssd1306_is_initialized()) {
        return;
    }

    uint8_t payload[2];
    payload[0] = 0x00U;
    payload[1] = cmd;
    (void)HAL_I2C_Master_Transmit(ssd1306_i2c, SSD1306_I2C_ADDRESS, payload, sizeof(payload), HAL_MAX_DELAY);
}

static void ssd1306_send_data(const uint8_t* data, size_t length) {
    if (!ssd1306_is_initialized() || data == NULL || length == 0U) {
        return;
    }

    uint8_t payload[17];
    payload[0] = 0x40U;

    while (length > 0U) {
        size_t chunk = (length > 16U) ? 16U : length;
        memcpy(&payload[1], data, chunk);
        (void)HAL_I2C_Master_Transmit(ssd1306_i2c, SSD1306_I2C_ADDRESS, payload, (uint16_t)(chunk + 1U), HAL_MAX_DELAY);
        data += chunk;
        length -= chunk;
    }
}
