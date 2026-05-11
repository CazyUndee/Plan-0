/*
 * state_graph.c - OpenSYS Kernel-Level State Graph Implementation
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

#include "state_graph.h"
#include "pmm.h"
#include "kheap.h"
#include "vga.h"
#include <stddef.h>
#include <stdint.h>

// Global state graph instance
static state_graph_t g_state_graph;

// Helper functions
static void k_memcpy(void* dst, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
}

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

// Simple spinlock
static void lock_acquire(volatile uint32_t* lock) {
    while (__sync_lock_test_and_set(lock, 1)) {
        __asm__ volatile ("pause");
    }
}

static void lock_release(volatile uint32_t* lock) {
    __sync_lock_release(lock);
}

// Initialize the state graph system
void state_graph_init(void) {
    // Clear everything
    for (int i = 0; i < 1024; i++) {
        g_state_graph.node_hash[i] = NULL;
    }
    
    for (int i = 0; i < NODE_TYPE_MAX; i++) {
        g_state_graph.root_nodes[i] = NULL;
    }
    
    // Allocate dedicated heap region (1MB)
    g_state_graph.heap_size = 1024 * 1024;
    g_state_graph.heap = (state_node_t*)kmalloc(g_state_graph.heap_size);
    g_state_graph.heap_used = 0;
    
    if (!g_state_graph.heap) {
        terminal_writestring_nl("FATAL: Failed to allocate state graph heap");
        while(1) __asm__ volatile("hlt");
    }
    
    g_state_graph.next_id = 1;
    g_state_graph.dirty_list = NULL;
    g_state_graph.observers = NULL;
    g_state_graph.lock = 0;
    
    // Create root nodes
    for (int i = 0; i < NODE_TYPE_MAX; i++) {
        node_id_t root_id = state_graph_create_node(i, NODE_ID_INVALID);
        if (root_id != NODE_ID_INVALID) {
            g_state_graph.root_nodes[i] = state_graph_get_node(root_id);
            g_state_graph.root_nodes[i]->flags |= NODE_FLAG_PERSISTENT;
        }
    }
}

// Get the global state graph
state_graph_t* state_graph_get(void) {
    return &g_state_graph;
}

// Allocate a node from the dedicated heap
static state_node_t* allocate_node(void) {
    if (g_state_graph.heap_used + sizeof(state_node_t) > g_state_graph.heap_size) {
        return NULL;  // Out of space
    }
    
    state_node_t* node = (state_node_t*)((uint8_t*)g_state_graph.heap + g_state_graph.heap_used);
    g_state_graph.heap_used += sizeof(state_node_t);
    
    // Clear the node
    for (size_t i = 0; i < sizeof(state_node_t); i++) {
        ((uint8_t*)node)[i] = 0;
    }
    
    return node;
}

// Hash map operations
static void hash_insert(state_node_t* node) {
    uint32_t hash = hash_node_id(node->id) % 1024;
    node->next_sibling_id = 0;  // Use this as next pointer in hash
    
    if (!g_state_graph.node_hash[hash]) {
        g_state_graph.node_hash[hash] = node;
    } else {
        // Insert at head of list
        state_node_t* current = g_state_graph.node_hash[hash];
        node->next_sibling_id = current->id;
        g_state_graph.node_hash[hash] = node;
    }
}

static state_node_t* hash_lookup(node_id_t id) {
    uint32_t hash = hash_node_id(id) % 1024;
    state_node_t* current = g_state_graph.node_hash[hash];
    
    while (current) {
        if (current->id == id) {
            return current;
        }
        
        // Traverse using next_sibling_id as hash chain pointer
        if (current->next_sibling_id == 0) break;
        current = state_graph_get_node(current->next_sibling_id);
    }
    
    return NULL;
}

// Create a new node
node_id_t state_graph_create_node(node_type_t type, node_id_t parent_id) {
    lock_acquire(&g_state_graph.lock);
    
    state_node_t* node = allocate_node();
    if (!node) {
        lock_release(&g_state_graph.lock);
        return NODE_ID_INVALID;
    }
    
    // Initialize node
    node->id = g_state_graph.next_id++;
    node->type = type;
    node->flags = NODE_FLAG_VISIBLE | NODE_FLAG_ENABLED;
    node->parent_id = parent_id;
    node->mft_record_id = 0;  // Will be set when synced to MFT
    
    // Add to hash map
    hash_insert(node);
    
    // Add to parent's children
    if (parent_id != NODE_ID_INVALID) {
        state_graph_add_child(parent_id, node->id);
    }
    
    // Notify observers
    state_graph_notify_observers(node->id, EVENT_CREATE);
    
    lock_release(&g_state_graph.lock);
    return node->id;
}

// Get a node by ID
state_node_t* state_graph_get_node(node_id_t node_id) {
    if (node_id == NODE_ID_INVALID) return NULL;
    return hash_lookup(node_id);
}

// Get root node for a type
state_node_t* state_graph_get_root(node_type_t type) {
    if (type >= NODE_TYPE_MAX) return NULL;
    return g_state_graph.root_nodes[type];
}

// Add child to parent
int state_graph_add_child(node_id_t parent_id, node_id_t child_id) {
    state_node_t* parent = state_graph_get_node(parent_id);
    state_node_t* child = state_graph_get_node(child_id);
    
    if (!parent || !child) return -1;
    
    // Insert at head of children list
    child->next_sibling_id = parent->first_child_id;
    parent->first_child_id = child_id;
    
    return 0;
}

// Set node geometry
int state_graph_set_geometry(node_id_t node_id, int x, int y, int w, int h) {
    lock_acquire(&g_state_graph.lock);
    
    state_node_t* node = state_graph_get_node(node_id);
    if (!node) {
        lock_release(&g_state_graph.lock);
        return -1;
    }
    
    node->geometry.x = x;
    node->geometry.y = y;
    node->geometry.width = w;
    node->geometry.height = h;
    
    node->flags |= NODE_FLAG_DIRTY;
    state_graph_notify_observers(node_id, EVENT_GEOMETRY);
    
    lock_release(&g_state_graph.lock);
    return 0;
}

// Set node text
int state_graph_set_text(node_id_t node_id, const char* text) {
    lock_acquire(&g_state_graph.lock);
    
    state_node_t* node = state_graph_get_node(node_id);
    if (!node) {
        lock_release(&g_state_graph.lock);
        return -1;
    }
    
    k_strcpy(node->text, text);
    node->flags |= NODE_FLAG_DIRTY;
    state_graph_notify_observers(node_id, EVENT_UPDATE);
    
    lock_release(&g_state_graph.lock);
    return 0;
}

// Set node value
int state_graph_set_value(node_id_t node_id, const char* value) {
    lock_acquire(&g_state_graph.lock);
    
    state_node_t* node = state_graph_get_node(node_id);
    if (!node) {
        lock_release(&g_state_graph.lock);
        return -1;
    }
    
    k_strcpy(node->value, value);
    node->flags |= NODE_FLAG_DIRTY;
    state_graph_notify_observers(node_id, EVENT_UPDATE);
    
    lock_release(&g_state_graph.lock);
    return 0;
}

// Set node flags
int state_graph_set_flags(node_id_t node_id, node_flags_t flags) {
    lock_acquire(&g_state_graph.lock);
    
    state_node_t* node = state_graph_get_node(node_id);
    if (!node) {
        lock_release(&g_state_graph.lock);
        return -1;
    }
    
    node->flags = flags;
    node->flags |= NODE_FLAG_DIRTY;
    state_graph_notify_observers(node_id, EVENT_UPDATE);
    
    lock_release(&g_state_graph.lock);
    return 0;
}

// Mark node as dirty (needs MFT sync)
int state_graph_mark_dirty(node_id_t node_id) {
    state_node_t* node = state_graph_get_node(node_id);
    if (!node) return -1;
    
    node->flags |= NODE_FLAG_DIRTY;
    return 0;
}

// Add observer
int state_graph_add_observer(uint32_t process_id, void (*notify)(node_id_t, uint32_t)) {
    observer_t* obs = (observer_t*)kmalloc(sizeof(observer_t));
    if (!obs) return -1;
    
    obs->process_id = process_id;
    obs->notify = notify;
    obs->next = g_state_graph.observers;
    g_state_graph.observers = obs;
    
    return 0;
}

// Remove observer
int state_graph_remove_observer(uint32_t process_id) {
    observer_t** current = &g_state_graph.observers;
    
    while (*current) {
        if ((*current)->process_id == process_id) {
            observer_t* to_remove = *current;
            *current = (*current)->next;
            kfree(to_remove);
            return 0;
        }
        current = &(*current)->next;
    }
    
    return -1;
}

// Notify all observers
void state_graph_notify_observers(node_id_t node_id, uint32_t event_mask) {
    observer_t* current = g_state_graph.observers;
    
    while (current) {
        if (current->notify) {
            current->notify(node_id, event_mask);
        }
        current = current->next;
    }
}

// Delete a node
int state_graph_delete_node(node_id_t node_id) {
    lock_acquire(&g_state_graph.lock);
    
    state_node_t* node = state_graph_get_node(node_id);
    if (!node) {
        lock_release(&g_state_graph.lock);
        return -1;
    }
    
    // Notify observers first
    state_graph_notify_observers(node_id, EVENT_DELETE);
    
    // Remove from hash (mark as deleted)
    node->id = NODE_ID_INVALID;
    
    lock_release(&g_state_graph.lock);
    return 0;
}
