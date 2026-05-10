/*
 * openinput.h - OpenSYS Input System Interface
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

#ifndef OPENINPUT_H
#define OPENINPUT_H

#include <stdint.h>

// Input device types
typedef enum {
    INPUT_DEVICE_PS2_KEYBOARD = 0,
    INPUT_DEVICE_USB_HID = 1,
    INPUT_DEVICE_MOUSE = 2,
    INPUT_DEVICE_TOUCHSCREEN = 3
} input_device_type_t;

// Input event types
typedef enum {
    INPUT_EVENT_KEY_PRESS = 0,
    INPUT_EVENT_KEY_RELEASE = 1,
    INPUT_EVENT_MOUSE_CLICK = 2,
    INPUT_EVENT_MOUSE_MOVE = 3,
    INPUT_EVENT_MOUSE_SCROLL = 4
} input_event_type_t;

// Input event structure
typedef struct {
    input_device_type_t device_type;
    input_event_type_t event_type;
    uint32_t key_code;
    int x, y;
    uint64_t timestamp;
} input_event_t;

// Core API
void openinput_init(void);
int openinput_has_event(void);
input_event_t openinput_get_event(void);
void openinput_flush_events(void);

// Device management
int openinput_register_device(input_device_type_t type, void (*callback)(input_event_t*));
int openinput_unregister_device(input_device_type_t type);

// Keyboard specific
int openkeyboard_init(void);
int openkeyboard_has_key(void);
char openkeyboard_getc(void);

// Mouse specific (future)
int openmouse_init(void);
int openmouse_has_event(void);
input_event_t openmouse_get_event(void);

#endif // OPENINPUT_H
