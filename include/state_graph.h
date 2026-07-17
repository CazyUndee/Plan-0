/*
 * state_graph.h - Plan 0 Kernel-Level State Graph
 *
 * Copyright (C) 2026 CazyUndee
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STATE_GRAPH_H
#define STATE_GRAPH_H

#include <stdint.h>
#include <stddef.h>

// Node types (expanded from ui_state.h)
typedef enum {
    NODE_TYPE_ROOT = 0,
    NODE_TYPE_WINDOW,
    NODE_TYPE_BUTTON,
    NODE_TYPE_INPUT,
    NODE_TYPE_TEXT,
    NODE_TYPE_CONTAINER,
    NODE_TYPE_MENU,
    NODE_TYPE_PROCESS,
    NODE_TYPE_FILESYSTEM,
    NODE_TYPE_MEMORY,
    NODE_TYPE_MAX
} node_type_t;

// Stable ID system (monotonically increasing)
typedef uint64_t node_id_t;
#define NODE_ID_INVALID 0

// Node state flags
typedef enum {
    NODE_FLAG_VISIBLE = (1 << 0),
    NODE_FLAG_FOCUSED = (1 << 1),
    NODE_FLAG_ENABLED = (1 << 2),
    NODE_FLAG_DIRTY = (1 << 3),    // Needs MFT sync
    NODE_FLAG_PERSISTENT = (1 << 4) // Survives reboot
} node_flags_t;

// Balanced tree node
typedef struct state_node {
    node_id_t id;
    node_type_t type;
    node_flags_t flags;
    
    // Hierarchy
    node_id_t parent_id;
    node_id_t first_child_id;
    node_id_t next_sibling_id;
    node_id_t prev_sibling_id;
    
    // Spatial data
    struct {
        int x, y, width, height;
    } geometry;
    
    // Content
    char text[256];
    char value[256];
    void* extra_data;
    
    // Tree balancing
    int8_t balance;
    struct state_node* left;
    struct state_node* right;
    
    // MFT linkage
    uint64_t mft_record_id;
    uint64_t last_sync_hash;
} state_node_t;

// Central registry
typedef struct {
    // Node storage (dedicated heap region)
    state_node_t* node_heap;
    size_t heap_size;
    size_t heap_used;
    
    // Fast lookup (hash map)
    state_node_t* node_hash[1024];
    
    // Root nodes
    state_node_t* root_nodes[NODE_TYPE_MAX];
    
    // ID generator
    node_id_t next_id;
    
    // Dirty list for MFT sync
    state_node_t* dirty_list;
    
    // Observers (GUI, CLI, etc.)
    struct observer* observers;
    
    // Lock for thread safety
    volatile uint32_t lock;
} state_graph_t;

// Observer pattern for notifications
typedef struct observer {
    uint32_t process_id;
    void (*notify)(node_id_t node_id, uint32_t event_mask);
    struct observer* next;
} observer_t;

// Event masks
#define EVENT_CREATE    (1 << 0)
#define EVENT_DELETE    (1 << 1)
#define EVENT_UPDATE    (1 << 2)
#define EVENT_FOCUS     (1 << 3)
#define EVENT_GEOMETRY  (1 << 4)

// Core API
void state_graph_init(void);
state_graph_t* state_graph_get(void);

// Node operations
node_id_t state_graph_create_node(node_type_t type, node_id_t parent_id);
int state_graph_delete_node(node_id_t node_id);
state_node_t* state_graph_get_node(node_id_t node_id);

// Tree operations
state_node_t* state_graph_get_root(node_type_t type);
state_node_t* state_graph_find_child(node_id_t parent_id, const char* name);
int state_graph_add_child(node_id_t parent_id, node_id_t child_id);

// State mutations
int state_graph_set_geometry(node_id_t node_id, int x, int y, int w, int h);
int state_graph_set_text(node_id_t node_id, const char* text);
int state_graph_set_value(node_id_t node_id, const char* value);
int state_graph_set_flags(node_id_t node_id, node_flags_t flags);

// Observer management
int state_graph_add_observer(uint32_t process_id, void (*notify)(node_id_t, uint32_t));
int state_graph_remove_observer(uint32_t process_id);
void state_graph_notify_observers(node_id_t node_id, uint32_t event_mask);

// MFT integration
int state_graph_mark_dirty(node_id_t node_id);
int state_graph_sync_to_mft(node_id_t node_id);
int state_graph_load_from_mft(void);

// Hash function for node IDs
static inline uint32_t hash_node_id(node_id_t id) {
    // Simple hash - could be improved
    return (uint32_t)(id ^ (id >> 32));
}

#endif // STATE_GRAPH_H
