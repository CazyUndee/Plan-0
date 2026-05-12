/*
 * intent_dispatcher.c - OpenSYS Intent Dispatcher Implementation
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

#include "intent_dispatcher.h"
#include "state_graph.h"
#include "vga.h"
#include "rtc.h"
#include <stddef.h>
#include <stdint.h>

// Global dispatcher instance
static intent_dispatcher_t g_dispatcher;

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

static int k_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *a - *b;
}

// Simple checksum for intent validation
static uint32_t calculate_checksum(intent_t* intent) {
    uint32_t sum = 0;
    uint8_t* bytes = (uint8_t*)intent;
    
    for (size_t i = 0; i < sizeof(intent_t) - sizeof(intent->checksum); i++) {
        sum += bytes[i];
    }
    
    return sum;
}

// Get current timestamp
static uint64_t get_timestamp(void) {
    rtc_time_t time;
    rtc_read_time(&time);
    
    // Simple timestamp (could be improved with proper timer)
    return (uint64_t)time.second + 
           (uint64_t)time.minute * 60 + 
           (uint64_t)time.hour * 3600;
}

// Initialize the intent dispatcher
void intent_dispatcher_init(void) {
    g_dispatcher.acl_list = NULL;
    g_dispatcher.acl_lock = 0;
    g_dispatcher.queue.count = 0;
    g_dispatcher.queue.capacity = 64;
    g_dispatcher.queue.last_flush = get_timestamp();
    g_dispatcher.intents_processed = 0;
    g_dispatcher.intents_failed = 0;
    g_dispatcher.next_sequence = 1;
    g_dispatcher.avg_process_time = 0;
    g_dispatcher.max_process_time = 0;
    
    // Add default permissions for system processes
    acl_add_permission(0, 0xFFFFFFFF);  // Kernel can do everything
    acl_add_permission(1, 0x0000FFFF);  // GUI process limited
    acl_add_permission(2, 0x0000FFFF);  // Shell process limited
}

// Validate an intent
int validate_intent(intent_t* intent) {
    if (!intent) return 0;
    
    // Check checksum
    uint32_t expected = calculate_checksum(intent);
    if (intent->checksum != expected) {
        return 0;
    }
    
    // Check intent type range
    if (intent->type < 100 || intent->type > 699) {
        return 0;
    }
    
    // Check target node exists (if specified)
    if (intent->target_id != NODE_ID_INVALID) {
        state_node_t* node = state_graph_get_node(intent->target_id);
        if (!node) {
            return 0;
        }
    }
    
    return 1;
}

// Check ACL permissions
int check_acl(uint32_t process_id, uint32_t intent_type) {
    acl_entry_t* current = g_dispatcher.acl_list;
    
    while (current) {
        if (current->process_id == process_id) {
            return (current->intent_mask & (1 << (intent_type / 32))) != 0;
        }
        current = current->next;
    }
    
    return 0;  // Default deny
}

// Apply intent to state graph
static error_t apply_intent_to_graph(intent_t* intent) {
    state_node_t* node = NULL;
    
    if (intent->target_id != NODE_ID_INVALID) {
        node = state_graph_get_node(intent->target_id);
        if (!node) return ERR_NODE_NOT_FOUND;
    }
    
    switch (intent->type) {
        case INTENT_CREATE_WINDOW: {
            node_id_t new_id = state_graph_create_node(NODE_TYPE_WINDOW, intent->target_id);
            if (new_id != NODE_ID_INVALID) {
                state_graph_set_text(new_id, intent->param1);
                state_graph_set_geometry(new_id, intent->int_param1, intent->int_param2, 400, 300);
                return ERR_SUCCESS;
            }
            return ERR_SYSTEM_ERROR;
        }
        
        case INTENT_MOVE_WINDOW: {
            if (node && node->type == NODE_TYPE_WINDOW) {
                state_graph_set_geometry(intent->target_id, 
                                     intent->int_param1, intent->int_param2,
                                     node->geometry.width, node->geometry.height);
                return ERR_SUCCESS;
            }
            return ERR_INVALID_PARAMS;
        }
        
        case INTENT_CLICK_NODE: {
            if (node) {
                // Handle click - could trigger callbacks
                state_graph_set_flags(intent->target_id, node->flags | NODE_FLAG_FOCUSED);
                return ERR_SUCCESS;
            }
            return ERR_NODE_NOT_FOUND;
        }
        
        case INTENT_SET_NODE_TEXT: {
            if (node) {
                state_graph_set_text(intent->target_id, intent->param1);
                return ERR_SUCCESS;
            }
            return ERR_NODE_NOT_FOUND;
        }
        
        case INTENT_DESTROY_WINDOW:
        case INTENT_DESTROY_NODE: {
            state_graph_delete_node(intent->target_id);
            return ERR_SUCCESS;
        }
        
        default:
            return ERR_INVALID_INTENT;
    }
}

// Core dispatch function
error_t intent_dispatch(intent_t* intent) {
    uint64_t start_time = get_timestamp();
    
    // 1. Validation
    if (!validate_intent(intent)) {
        g_dispatcher.intents_failed++;
        return ERR_INVALID_INTENT;
    }
    
    // 2. ACL check
    if (!check_acl(intent->source_process_id, intent->type)) {
        g_dispatcher.intents_failed++;
        return ERR_UNAUTHORIZED;
    }
    
    // 3. Journaling (write-ahead log)
    // TODO: Implement MFT journaling
    // mft_journal_append(intent);
    
    // 4. Apply to state graph
    error_t result = apply_intent_to_graph(intent);
    
    // 5. Notify observers (handled by state graph)
    
    // 6. Update statistics
    uint64_t end_time = get_timestamp();
    uint64_t process_time = end_time - start_time;
    
    g_dispatcher.intents_processed++;
    if (process_time > g_dispatcher.max_process_time) {
        g_dispatcher.max_process_time = process_time;
    }
    
    // Update average (simple moving average)
    g_dispatcher.avg_process_time = 
        (g_dispatcher.avg_process_time * 9 + process_time) / 10;
    
    return result;
}

// Intent builders
intent_t intent_create_window(node_id_t parent_id, const char* title) {
    intent_t intent = {0};
    intent.type = INTENT_CREATE_WINDOW;
    intent.target_id = parent_id;
    k_strcpy(intent.param1, title);
    intent.int_param1 = 100;  // Default x
    intent.int_param2 = 100;  // Default y
    intent.source_process_id = 0;  // Current process
    intent.timestamp = get_timestamp();
    intent.sequence_number = g_dispatcher.next_sequence++;
    intent.priority = 1;  // Normal priority
    intent.checksum = calculate_checksum(&intent);
    
    return intent;
}

intent_t intent_move_window(node_id_t window_id, int x, int y) {
    intent_t intent = {0};
    intent.type = INTENT_MOVE_WINDOW;
    intent.target_id = window_id;
    intent.int_param1 = x;
    intent.int_param2 = y;
    intent.source_process_id = 0;
    intent.timestamp = get_timestamp();
    intent.sequence_number = g_dispatcher.next_sequence++;
    intent.priority = 1;
    intent.checksum = calculate_checksum(&intent);
    
    return intent;
}

intent_t intent_click_node(node_id_t node_id) {
    intent_t intent = {0};
    intent.type = INTENT_CLICK_NODE;
    intent.target_id = node_id;
    intent.source_process_id = 0;
    intent.timestamp = get_timestamp();
    intent.sequence_number = g_dispatcher.next_sequence++;
    intent.priority = 2;  // High priority for interactions
    intent.checksum = calculate_checksum(&intent);
    
    return intent;
}

intent_t intent_set_node_text(node_id_t node_id, const char* text) {
    intent_t intent = {0};
    intent.type = INTENT_SET_NODE_TEXT;
    intent.target_id = node_id;
    k_strcpy(intent.param1, text);
    intent.source_process_id = 0;
    intent.timestamp = get_timestamp();
    intent.sequence_number = g_dispatcher.next_sequence++;
    intent.priority = 1;
    intent.checksum = calculate_checksum(&intent);
    
    return intent;
}

// ACL management
int acl_add_permission(uint32_t process_id, uint32_t intent_mask) {
    acl_entry_t* entry = (acl_entry_t*)kmalloc(sizeof(acl_entry_t));
    if (!entry) return -1;
    
    entry->process_id = process_id;
    entry->intent_mask = intent_mask;
    entry->next = g_dispatcher.acl_list;
    g_dispatcher.acl_list = entry;
    
    return 0;
}

int acl_remove_permission(uint32_t process_id) {
    acl_entry_t** current = &g_dispatcher.acl_list;
    
    while (*current) {
        if ((*current)->process_id == process_id) {
            acl_entry_t* to_remove = *current;
            *current = (*current)->next;
            kfree(to_remove);
            return 0;
        }
        current = &(*current)->next;
    }
    
    return -1;
}

// Simple memcpy implementation
static void k_memcpy(void* dst, const void* src, size_t n) {
    char* d = (char*)dst;
    const char* s = (const char*)src;
    while (n--) *d++ = *s++;
}

// System call interface
error_t sys_intent_dispatch(void* intent_ptr) {
    if (!intent_ptr) return ERR_INVALID_PARAMS;

    // Copy intent from user space (simplified)
    intent_t intent;
    k_memcpy(&intent, intent_ptr, sizeof(intent_t));
    
    // Set source process ID (would get from current process)
    intent.source_process_id = 1;  // TODO: Get actual process ID
    
    return intent_dispatch(&intent);
}

// Get statistics
void intent_get_stats(uint64_t* processed, uint64_t* failed, uint64_t* avg_time) {
    if (processed) *processed = g_dispatcher.intents_processed;
    if (failed) *failed = g_dispatcher.intents_failed;
    if (avg_time) *avg_time = g_dispatcher.avg_process_time;
}
