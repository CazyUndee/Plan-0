/*
 * ui_state.h - OpenSYS Unified UI State System
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

#ifndef UI_STATE_H
#define UI_STATE_H

#include <stdint.h>
#include <stddef.h>
#include "intent_dispatcher.h"

// Maximum limits
#define MAX_WINDOWS 64
#define MAX_NODES 1024
#define MAX_PROCESSES 64
#define MAX_ID_LENGTH 64

// Node types
typedef enum {
    NODE_TYPE_WINDOW,
    NODE_TYPE_BUTTON,
    NODE_TYPE_INPUT,
    NODE_TYPE_TEXT,
    NODE_TYPE_CONTAINER,
    NODE_TYPE_MENU,
    NODE_TYPE_PROCESS
} node_type_t;

// Window state
typedef struct {
    char id[MAX_ID_LENGTH];
    char title[MAX_ID_LENGTH];
    int x, y;
    int width, height;
    int visible;
    int focused;
    uint32_t process_id;
} window_t;

// UI node state
typedef struct {
    char id[MAX_ID_LENGTH];
    node_type_t type;
    char parent_id[MAX_ID_LENGTH];
    int x, y;
    int width, height;
    int visible;
    int focused;
    int enabled;
    char text[256];
    char value[256];
    void* extra_data;
} node_t;

// Process state
typedef struct {
    uint32_t pid;
    char name[MAX_ID_LENGTH];
    int active;
    uint32_t main_window_id;
} ui_process_t;

// Focus state
typedef struct {
    char window_id[MAX_ID_LENGTH];
    char node_id[MAX_ID_LENGTH];
} focus_state_t;

// System state graph
typedef struct {
    window_t windows[MAX_WINDOWS];
    node_t nodes[MAX_NODES];
    ui_process_t processes[MAX_PROCESSES];
    focus_state_t focus;
    int window_count;
    int node_count;
    int process_count;
} ui_state_t;

// Core API
void ui_state_init(void);
ui_state_t* ui_state_get(void);

// State mutations
int ui_state_execute_intent(intent_t* intent);
int ui_state_create_window(const char* id, const char* title, int x, int y, int w, int h);
int ui_state_create_node(const char* id, node_type_t type, const char* parent_id);
int ui_state_delete_node(const char* id);
int ui_state_set_focus(const char* window_id, const char* node_id);

// State queries
window_t* ui_state_get_window(const char* id);
node_t* ui_state_get_node(const char* id);
ui_process_t* ui_state_get_process(uint32_t pid);
int ui_state_get_focused_window(void);
int ui_state_get_focused_node(void);

#endif // UI_STATE_H
