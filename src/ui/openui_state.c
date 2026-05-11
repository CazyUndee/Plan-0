/*
 * ui_state.c - OpenSYS Unified UI State Implementation
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

#include "ui_state.h"
#include "vga.h"
#include <stddef.h>
#include <stdint.h>

// Global state instance
static ui_state_t g_state;

// Helper functions
static int k_strlen(const char* s) { 
    int n = 0; 
    while (*s++) n++; 
    return n; 
}

static int k_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *a - *b;
}

static void k_strcpy(char* dst, const char* src) {
    while (*src) *dst++ = *src++;
    *dst = 0;
}

// Initialize the state system
void ui_state_init(void) {
    // Clear everything
    for (int i = 0; i < MAX_WINDOWS; i++) {
        g_state.windows[i].id[0] = 0;
    }
    for (int i = 0; i < MAX_NODES; i++) {
        g_state.nodes[i].id[0] = 0;
    }
    for (int i = 0; i < MAX_PROCESSES; i++) {
        g_state.processes[i].name[0] = 0;
    }
    
    g_state.window_count = 0;
    g_state.node_count = 0;
    g_state.process_count = 0;
    g_state.focus.window_id[0] = 0;
    g_state.focus.node_id[0] = 0;
}

// Get the global state
ui_state_t* ui_state_get(void) {
    return &g_state;
}

// Find window by ID
static window_t* find_window(const char* id) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (k_strcmp(g_state.windows[i].id, id) == 0) {
            return &g_state.windows[i];
        }
    }
    return NULL;
}

// Find node by ID
static node_t* find_node(const char* id) {
    for (int i = 0; i < MAX_NODES; i++) {
        if (k_strcmp(g_state.nodes[i].id, id) == 0) {
            return &g_state.nodes[i];
        }
    }
    return NULL;
}

// Execute an intent (state mutation)
int ui_state_execute_intent(intent_t* intent) {
    if (!intent) return -1;
    
    switch (intent->type) {
        case INTENT_OPEN_WINDOW: {
            window_t* win = find_window(intent->target_id);
            if (win) {
                k_strcpy(win->title, intent->param1);
                win->visible = 1;
                return 0;
            }
            return -1;
        }
        
        case INTENT_CLOSE_WINDOW: {
            window_t* win = find_window(intent->target_id);
            if (win) {
                win->visible = 0;
                win->id[0] = 0;  // Mark as deleted
                g_state.window_count--;
                return 0;
            }
            return -1;
        }
        
        case INTENT_SET_WINDOW_POSITION: {
            window_t* win = find_window(intent->target_id);
            if (win) {
                win->x = intent->int_param1;
                win->y = intent->int_param2;
                return 0;
            }
            return -1;
        }
        
        case INTENT_SET_WINDOW_SIZE: {
            window_t* win = find_window(intent->target_id);
            if (win) {
                win->width = intent->int_param1;
                win->height = intent->int_param2;
                return 0;
            }
            return -1;
        }
        
        case INTENT_SET_WINDOW_VISIBILITY: {
            window_t* win = find_window(intent->target_id);
            if (win) {
                win->visible = intent->int_param1;
                return 0;
            }
            return -1;
        }
        
        case INTENT_CREATE_NODE: {
            node_t* node = find_node(intent->target_id);
            if (node) {
                node->type = (node_type_t)intent->int_param1;
                k_strcpy(node->parent_id, intent->param1);
                node->visible = 1;
                node->enabled = 1;
                return 0;
            }
            return -1;
        }
        
        case INTENT_DELETE_NODE: {
            node_t* node = find_node(intent->target_id);
            if (node) {
                node->id[0] = 0;  // Mark as deleted
                g_state.node_count--;
                return 0;
            }
            return -1;
        }
        
        case INTENT_SET_NODE_POSITION: {
            node_t* node = find_node(intent->target_id);
            if (node) {
                node->x = intent->int_param1;
                node->y = intent->int_param2;
                return 0;
            }
            return -1;
        }
        
        case INTENT_SET_NODE_TEXT: {
            node_t* node = find_node(intent->target_id);
            if (node) {
                k_strcpy(node->text, intent->param1);
                return 0;
            }
            return -1;
        }
        
        case INTENT_SET_NODE_VALUE: {
            node_t* node = find_node(intent->target_id);
            if (node) {
                k_strcpy(node->value, intent->param1);
                return 0;
            }
            return -1;
        }
        
        case INTENT_SET_NODE_VISIBILITY: {
            node_t* node = find_node(intent->target_id);
            if (node) {
                node->visible = intent->int_param1;
                return 0;
            }
            return -1;
        }
        
        case INTENT_CLICK_NODE: {
            // Click is handled by event system, here we just mark it
            node_t* node = find_node(intent->target_id);
            if (node && node->enabled && node->visible) {
                // Could trigger callbacks here
                return 0;
            }
            return -1;
        }
        
        case INTENT_SET_FOCUS: {
            k_strcpy(g_state.focus.window_id, intent->param1);
            k_strcpy(g_state.focus.node_id, intent->param2);
            
            // Update window focus flags
            window_t* win = find_window(intent->param1);
            if (win) win->focused = 1;
            
            // Update node focus flags
            node_t* node = find_node(intent->param2);
            if (node) node->focused = 1;
            
            return 0;
        }
        
        case INTENT_START_PROCESS: {
            for (int i = 0; i < MAX_PROCESSES; i++) {
                if (g_state.processes[i].name[0] == 0) {
                    g_state.processes[i].pid = intent->int_param1;
                    k_strcpy(g_state.processes[i].name, intent->param1);
                    g_state.processes[i].active = 1;
                    g_state.processes[i].main_window_id = 0;
                    g_state.process_count++;
                    return 0;
                }
            }
            return -1;
        }
        
        case INTENT_STOP_PROCESS: {
            for (int i = 0; i < MAX_PROCESSES; i++) {
                if (g_state.processes[i].pid == intent->int_param1) {
                    g_state.processes[i].name[0] = 0;
                    g_state.processes[i].active = 0;
                    g_state.process_count--;
                    return 0;
                }
            }
            return -1;
        }
        
        default:
            return -1;
    }
}

// Create a window
int ui_state_create_window(const char* id, const char* title, int x, int y, int w, int h) {
    if (g_state.window_count >= MAX_WINDOWS) return -1;
    
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (g_state.windows[i].id[0] == 0) {
            k_strcpy(g_state.windows[i].id, id);
            k_strcpy(g_state.windows[i].title, title);
            g_state.windows[i].x = x;
            g_state.windows[i].y = y;
            g_state.windows[i].width = w;
            g_state.windows[i].height = h;
            g_state.windows[i].visible = 1;
            g_state.windows[i].focused = 0;
            g_state.windows[i].process_id = 0;
            g_state.window_count++;
            return i;
        }
    }
    return -1;
}

// Create a node
int ui_state_create_node(const char* id, node_type_t type, const char* parent_id) {
    if (g_state.node_count >= MAX_NODES) return -1;
    
    for (int i = 0; i < MAX_NODES; i++) {
        if (g_state.nodes[i].id[0] == 0) {
            k_strcpy(g_state.nodes[i].id, id);
            g_state.nodes[i].type = type;
            k_strcpy(g_state.nodes[i].parent_id, parent_id);
            g_state.nodes[i].x = 0;
            g_state.nodes[i].y = 0;
            g_state.nodes[i].width = 0;
            g_state.nodes[i].height = 0;
            g_state.nodes[i].visible = 1;
            g_state.nodes[i].focused = 0;
            g_state.nodes[i].enabled = 1;
            g_state.nodes[i].text[0] = 0;
            g_state.nodes[i].value[0] = 0;
            g_state.nodes[i].extra_data = NULL;
            g_state.node_count++;
            return i;
        }
    }
    return -1;
}

// Delete a node
int ui_state_delete_node(const char* id) {
    node_t* node = find_node(id);
    if (node) {
        node->id[0] = 0;
        g_state.node_count--;
        return 0;
    }
    return -1;
}

// Set focus
int ui_state_set_focus(const char* window_id, const char* node_id) {
    k_strcpy(g_state.focus.window_id, window_id);
    k_strcpy(g_state.focus.node_id, node_id);
    return 0;
}

// Get window by ID
window_t* ui_state_get_window(const char* id) {
    return find_window(id);
}

// Get node by ID
node_t* ui_state_get_node(const char* id) {
    return find_node(id);
}

// Get process by PID
process_t* ui_state_get_process(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (g_state.processes[i].pid == pid && g_state.processes[i].active) {
            return &g_state.processes[i];
        }
    }
    return NULL;
}

// Intent builders
intent_t intent_open_window(const char* id, const char* title) {
    intent_t intent = {0};
    intent.type = INTENT_OPEN_WINDOW;
    k_strcpy(intent.target_id, id);
    k_strcpy(intent.param1, title);
    return intent;
}

intent_t intent_close_window(const char* id) {
    intent_t intent = {0};
    intent.type = INTENT_CLOSE_WINDOW;
    k_strcpy(intent.target_id, id);
    return intent;
}

intent_t intent_set_window_position(const char* id, int x, int y) {
    intent_t intent = {0};
    intent.type = INTENT_SET_WINDOW_POSITION;
    k_strcpy(intent.target_id, id);
    intent.int_param1 = x;
    intent.int_param2 = y;
    return intent;
}

intent_t intent_click_node(const char* id) {
    intent_t intent = {0};
    intent.type = INTENT_CLICK_NODE;
    k_strcpy(intent.target_id, id);
    return intent;
}

intent_t intent_set_focus_window(const char* id) {
    intent_t intent = {0};
    intent.type = INTENT_SET_FOCUS;
    k_strcpy(intent.target_id, id);
    k_strcpy(intent.param1, id);
    return intent;
}

intent_t intent_set_focus_node(const char* id) {
    intent_t intent = {0};
    intent.type = INTENT_SET_FOCUS;
    k_strcpy(intent.target_id, id);
    k_strcpy(intent.param2, id);
    return intent;
}
