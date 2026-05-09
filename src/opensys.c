/*
 * opensys.c - OpenSYS OpenLibC Implementation
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

#include "../include/opensys.h"
#include "../include/intent_dispatcher.h"
#include "../include/state_graph.h"
#include <stddef.h>
#include <stdint.h>

// Helper functions
static int k_strlen(const char* s) {
    int n = 0;
    while (*s++) n++;
    return n;
}

static void k_strcpy(char* dst, const char* src) {
    while (*src) *dst++ = *src++;
    *dst = 0;
}

// ============================================================================
// High-level OpenLibC API Implementation
// ============================================================================

// Window management
os_error_t os_open_window(const char* title, int x, int y, int width, int height, os_node_id_t* window_id) {
    if (!title || !window_id) return OS_ERROR_INVALID_PARAM;
    
    // Create intent
    os_intent_t intent = {0};
    intent.type = 100;  // INTENT_CREATE_WINDOW
    intent.int_param1 = x;
    intent.int_param2 = y;
    k_strcpy(intent.param1, title);
    
    // Dispatch intent
    os_error_t result = os_sys_intent_dispatch(&intent);
    if (result == OS_SUCCESS) {
        // Get the created window ID (simplified - would be returned by intent)
        *window_id = 1;  // TODO: Get actual ID from response
    }
    
    return result;
}

os_error_t os_close_window(os_node_id_t window_id) {
    if (window_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 101;  // INTENT_DESTROY_WINDOW
    intent.target_id = window_id;
    
    return os_sys_intent_dispatch(&intent);
}

os_error_t os_move_window(os_node_id_t window_id, int x, int y) {
    if (window_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 102;  // INTENT_MOVE_WINDOW
    intent.target_id = window_id;
    intent.int_param1 = x;
    intent.int_param2 = y;
    
    return os_sys_intent_dispatch(&intent);
}

os_error_t os_resize_window(os_node_id_t window_id, int width, int height) {
    if (window_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 103;  // INTENT_RESIZE_WINDOW
    intent.target_id = window_id;
    intent.int_param1 = width;
    intent.int_param2 = height;
    
    return os_sys_intent_dispatch(&intent);
}

os_error_t os_show_window(os_node_id_t window_id) {
    if (window_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 104;  // INTENT_SHOW_WINDOW
    intent.target_id = window_id;
    
    return os_sys_intent_dispatch(&intent);
}

os_error_t os_hide_window(os_node_id_t window_id) {
    if (window_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 105;  // INTENT_HIDE_WINDOW
    intent.target_id = window_id;
    
    return os_sys_intent_dispatch(&intent);
}

os_error_t os_focus_window(os_node_id_t window_id) {
    if (window_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 106;  // INTENT_FOCUS_WINDOW
    intent.target_id = window_id;
    
    return os_sys_intent_dispatch(&intent);
}

// Node management
os_error_t os_create_button(os_node_id_t parent_id, const char* text, int x, int y, int width, int height, os_node_id_t* button_id) {
    if (!text || !button_id || parent_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 200;  // INTENT_CREATE_NODE
    intent.target_id = parent_id;
    intent.int_param1 = 1;  // NODE_TYPE_BUTTON
    intent.int_param2 = 1;  // Create flag
    k_strcpy(intent.param1, text);
    intent.int_param1 = x;
    intent.int_param2 = y;
    
    os_error_t result = os_sys_intent_dispatch(&intent);
    if (result == OS_SUCCESS) {
        *button_id = 2;  // TODO: Get actual ID
    }
    
    return result;
}

os_error_t os_create_input(os_node_id_t parent_id, const char* default_text, int x, int y, int width, int height, os_node_id_t* input_id) {
    if (!input_id || parent_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 200;  // INTENT_CREATE_NODE
    intent.target_id = parent_id;
    intent.int_param1 = 3;  // NODE_TYPE_INPUT
    k_strcpy(intent.param1, default_text ? default_text : "");
    
    os_error_t result = os_sys_intent_dispatch(&intent);
    if (result == OS_SUCCESS) {
        *input_id = 3;  // TODO: Get actual ID
    }
    
    return result;
}

os_error_t os_create_text(os_node_id_t parent_id, const char* text, int x, int y, os_node_id_t* text_id) {
    if (!text || !text_id || parent_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 200;  // INTENT_CREATE_NODE
    intent.target_id = parent_id;
    intent.int_param1 = 4;  // NODE_TYPE_TEXT
    k_strcpy(intent.param1, text);
    
    os_error_t result = os_sys_intent_dispatch(&intent);
    if (result == OS_SUCCESS) {
        *text_id = 4;  // TODO: Get actual ID
    }
    
    return result;
}

os_error_t os_set_node_text(os_node_id_t node_id, const char* text) {
    if (!text || node_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 204;  // INTENT_SET_NODE_TEXT
    intent.target_id = node_id;
    k_strcpy(intent.param1, text);
    
    return os_sys_intent_dispatch(&intent);
}

os_error_t os_set_node_value(os_node_id_t node_id, const char* value) {
    if (!value || node_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 205;  // INTENT_SET_NODE_VALUE
    intent.target_id = node_id;
    k_strcpy(intent.param1, value);
    
    return os_sys_intent_dispatch(&intent);
}

os_error_t os_get_node_text(os_node_id_t node_id, char* buffer, size_t buffer_size) {
    if (!buffer || node_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_node_info_t info;
    os_error_t result = os_sys_get_node_info(node_id, &info);
    if (result == OS_SUCCESS) {
        k_strcpy(buffer, info.text);
    }
    
    return result;
}

os_error_t os_get_node_value(os_node_id_t node_id, char* buffer, size_t buffer_size) {
    if (!buffer || node_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_node_info_t info;
    os_error_t result = os_sys_get_node_info(node_id, &info);
    if (result == OS_SUCCESS) {
        k_strcpy(buffer, info.value);
    }
    
    return result;
}

// Interaction
os_error_t os_click_node(os_node_id_t node_id) {
    if (node_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 300;  // INTENT_CLICK_NODE
    intent.target_id = node_id;
    
    return os_sys_intent_dispatch(&intent);
}

os_error_t os_send_key(os_node_id_t node_id, uint32_t key_code) {
    if (node_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 301;  // INTENT_KEY_PRESS
    intent.target_id = node_id;
    intent.int_param1 = key_code;
    
    return os_sys_intent_dispatch(&intent);
}

os_error_t os_send_text(os_node_id_t node_id, const char* text) {
    if (!text || node_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    os_intent_t intent = {0};
    intent.type = 302;  // INTENT_TEXT_INPUT
    intent.target_id = node_id;
    k_strcpy(intent.param1, text);
    
    return os_sys_intent_dispatch(&intent);
}

// System information
os_error_t os_list_windows(os_node_id_t* windows, size_t* count) {
    if (!windows || !count) return OS_ERROR_INVALID_PARAM;
    
    // TODO: Implement system call to list windows
    *count = 0;
    return OS_SUCCESS;
}

os_error_t os_list_children(os_node_id_t parent_id, os_node_id_t* children, size_t* count) {
    if (!children || !count || parent_id == OS_NODE_INVALID) return OS_ERROR_INVALID_PARAM;
    
    // TODO: Implement system call to list children
    *count = 0;
    return OS_SUCCESS;
}

// ============================================================================
// Low-level intent interface
// ============================================================================

os_error_t os_create_intent(uint32_t type, os_node_id_t target_id, os_intent_t* intent) {
    if (!intent) return OS_ERROR_INVALID_PARAM;
    
    // Clear intent
    for (size_t i = 0; i < sizeof(os_intent_t); i++) {
        ((uint8_t*)intent)[i] = 0;
    }
    
    intent->type = type;
    intent->target_id = target_id;
    intent->source_process_id = 1;  // TODO: Get actual process ID
    
    return OS_SUCCESS;
}

os_error_t os_set_intent_param(os_intent_t* intent, const char* param1, const char* param2, int int1, int int2) {
    if (!intent) return OS_ERROR_INVALID_PARAM;
    
    if (param1) k_strcpy(intent->param1, param1);
    if (param2) k_strcpy(intent->param2, param2);
    intent->int_param1 = int1;
    intent->int_param2 = int2;
    
    return OS_SUCCESS;
}

os_error_t os_dispatch_intent(os_intent_t* intent) {
    if (!intent) return OS_ERROR_INVALID_PARAM;
    
    return os_sys_intent_dispatch(intent);
}

// ============================================================================
// Example usage functions
// ============================================================================

// Example: Create a simple window with a button
os_error_t os_create_simple_window(const char* title, os_node_id_t* window_id) {
    os_node_id_t win_id, btn_id;
    os_error_t result;
    
    // Create window
    result = os_open_window(title, 100, 100, 400, 300, &win_id);
    if (result != OS_SUCCESS) return result;
    
    // Create button
    result = os_create_button(win_id, "Click Me", 150, 200, 100, 30, &btn_id);
    if (result != OS_SUCCESS) return result;
    
    if (window_id) *window_id = win_id;
    return OS_SUCCESS;
}

// Example: Move window to center of screen
os_error_t os_center_window(os_node_id_t window_id, int screen_width, int screen_height) {
    os_node_info_t info;
    os_error_t result = os_sys_get_node_info(window_id, &info);
    if (result != OS_SUCCESS) return result;
    
    int x = (screen_width - info.geometry.width) / 2;
    int y = (screen_height - info.geometry.height) / 2;
    
    return os_move_window(window_id, x, y);
}
