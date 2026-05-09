/*
 * opensys.h - OpenSYS System Call Interface (OpenLibC)
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

#ifndef OPENSYS_H
#define OPENSYS_H

#include <stdint.h>

// System call numbers
#define SYS_INTENT_DISPATCH  0x100
#define SYS_GET_NODE_ID     0x101
#define SYS_GET_NODE_INFO   0x102
#define SYS_WAIT_FOR_EVENT  0x103
#define SYS_GET_STATS       0x104

// Node ID types
typedef uint64_t os_node_id_t;
#define OS_NODE_INVALID 0

// OpenLibC error codes
typedef enum {
    OS_SUCCESS = 0,
    OS_ERROR_INVALID_PARAM = -1,
    OS_ERROR_NOT_FOUND = -2,
    OS_ERROR_PERMISSION_DENIED = -3,
    OS_ERROR_SYSTEM_ERROR = -4
} os_error_t;

// Forward declarations
typedef struct os_intent os_intent_t;
typedef struct os_node_info os_node_info_t;

// Intent structure (user-space compatible)
struct os_intent {
    uint32_t type;
    os_node_id_t target_id;
    
    char param1[256];
    char param2[256];
    int int_param1;
    int int_param2;
    
    uint32_t source_process_id;
    uint64_t timestamp;
    uint32_t sequence_number;
    uint32_t checksum;
    uint8_t priority;
    
    void* data;
    size_t data_size;
};

// Node information structure
struct os_node_info {
    os_node_id_t id;
    uint32_t type;
    uint32_t flags;
    
    struct {
        int x, y, width, height;
    } geometry;
    
    char text[256];
    char value[256];
    
    os_node_id_t parent_id;
    os_node_id_t first_child_id;
    os_node_id_t next_sibling_id;
};

// Event structure for waiting
typedef struct {
    os_node_id_t node_id;
    uint32_t event_mask;
    uint64_t timeout;
} os_event_t;

// ============================================================================
// OpenLibC API - High-level natural language interface
// ============================================================================

// Window management
os_error_t os_open_window(const char* title, int x, int y, int width, int height, os_node_id_t* window_id);
os_error_t os_close_window(os_node_id_t window_id);
os_error_t os_move_window(os_node_id_t window_id, int x, int y);
os_error_t os_resize_window(os_node_id_t window_id, int width, int height);
os_error_t os_show_window(os_node_id_t window_id);
os_error_t os_hide_window(os_node_id_t window_id);
os_error_t os_focus_window(os_node_id_t window_id);

// Node management (UI components)
os_error_t os_create_button(os_node_id_t parent_id, const char* text, int x, int y, int width, int height, os_node_id_t* button_id);
os_error_t os_create_input(os_node_id_t parent_id, const char* default_text, int x, int y, int width, int height, os_node_id_t* input_id);
os_error_t os_create_text(os_node_id_t parent_id, const char* text, int x, int y, os_node_id_t* text_id);
os_error_t os_set_node_text(os_node_id_t node_id, const char* text);
os_error_t os_set_node_value(os_node_id_t node_id, const char* value);
os_error_t os_get_node_text(os_node_id_t node_id, char* buffer, size_t buffer_size);
os_error_t os_get_node_value(os_node_id_t node_id, char* buffer, size_t buffer_size);

// Interaction
os_error_t os_click_node(os_node_id_t node_id);
os_error_t os_send_key(os_node_id_t node_id, uint32_t key_code);
os_error_t os_send_text(os_node_id_t node_id, const char* text);

// System information
os_error_t os_list_windows(os_node_id_t* windows, size_t* count);
os_error_t os_list_children(os_node_id_t parent_id, os_node_id_t* children, size_t* count);
os_error_t os_get_node_info(os_node_id_t node_id, os_node_info_t* info);

// Event handling
os_error_t os_wait_for_event(os_event_t* events, size_t event_count, os_event_t* triggered_event);
os_error_t os_register_observer(void (*callback)(os_node_id_t, uint32_t));

// ============================================================================
// Low-level intent interface (for advanced usage)
// ============================================================================

// Intent creation and dispatch
os_error_t os_create_intent(uint32_t type, os_node_id_t target_id, os_intent_t* intent);
os_error_t os_set_intent_param(os_intent_t* intent, const char* param1, const char* param2, int int1, int int2);
os_error_t os_dispatch_intent(os_intent_t* intent);

// ============================================================================
// System calls (internal use)
// ============================================================================

// System call wrapper
static inline os_error_t os_syscall(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    os_error_t result;
    __asm__ volatile (
        "movq %1, %%rax\n"
        "movq %2, %%rdi\n"
        "movq %3, %%rsi\n"
        "movq %4, %%rdx\n"
        "int $0x80\n"
        "movq %%rax, %0\n"
        : "=r"(result)
        : "r"(syscall_num), "r"(arg1), "r"(arg2), "r"(arg3)
        : "rax", "rdi", "rsi", "rdx", "memory"
    );
    return result;
}

// System call implementations
static inline os_error_t os_sys_intent_dispatch(os_intent_t* intent) {
    return os_syscall(SYS_INTENT_DISPATCH, (uint64_t)intent, 0, 0);
}

static inline os_error_t os_sys_get_node_id(const char* name, os_node_id_t* node_id) {
    return os_syscall(SYS_GET_NODE_ID, (uint64_t)name, (uint64_t)node_id, 0);
}

static inline os_error_t os_sys_get_node_info(os_node_id_t node_id, os_node_info_t* info) {
    return os_syscall(SYS_GET_NODE_INFO, (uint64_t)node_id, (uint64_t)info, 0);
}

static inline os_error_t os_sys_wait_for_event(os_event_t* events, size_t count, os_event_t* triggered) {
    return os_syscall(SYS_WAIT_FOR_EVENT, (uint64_t)events, (uint64_t)count, (uint64_t)triggered);
}

#endif // OPENSYS_H
