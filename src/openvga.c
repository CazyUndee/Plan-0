/*
 * openvga.c - OpenSYS VGA Display Interface
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

#include "../include/openvga.h"
#include "../include/vga.h"
#include <stddef.h>
#include <stdint.h>

// External VGA functions
extern void terminal_initialize(void);
extern void terminal_writestring(const char* str);
extern void terminal_putchar(char c);
extern void terminal_clear(void);
extern void terminal_set_color(uint8_t fg, uint8_t bg);

// Initialize OpenVGA system
void openvga_init(void) {
    terminal_writestring("[OPENVGA] Initializing Display System...\n");
    
    // Initialize terminal
    terminal_initialize();
    
    terminal_writestring("  VGA text mode 80x25\n");
    terminal_writestring("  16 colors available\n");
    
    terminal_writestring("[OPENVGA] Display System Ready!\n");
}

// Clear screen
void openclear_screen(void) {
    terminal_clear();
}

// Set text color
void openset_color(uint8_t fg, uint8_t bg) {
    terminal_set_color(fg, bg);
}

// Get display information
void openvga_get_info(openvga_info_t* info) {
    if (!info) return;
    
    info->width = 80;
    info->height = 25;
    info->colors = 16;
    info->mode = 0;  // Text mode
}
