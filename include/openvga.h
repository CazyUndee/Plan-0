/*
 * openvga.h - OpenSYS Display Interface
 *
 * Copyright (C) 2026 CazyUndee
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef OPENVGA_H
#define OPENVGA_H

#include <stdint.h>

// Display modes
typedef enum {
    DISPLAY_MODE_TEXT = 0,
    DISPLAY_MODE_GRAPHICS = 1
} display_mode_t;

// VGA colors
typedef enum {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_LIGHT_BROWN = 14,
    COLOR_WHITE = 15
} vga_color_t;

// Display information
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t colors;
    display_mode_t mode;
} openvga_info_t;

// Core API
void openvga_init(void);
void openclear_screen(void);
void openset_color(vga_color_t fg, vga_color_t bg);
void openvga_get_info(openvga_info_t* info);

// Text output
void openprint(const char* str);
void openprint_char(char c);
void openprintln(const char* str);

// Graphics operations (future)
void openpixel(uint16_t x, uint16_t y, vga_color_t color);
void openline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, vga_color_t color);

#endif // OPENVGA_H
