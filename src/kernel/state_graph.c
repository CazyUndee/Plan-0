/*
 * state_graph.c - Plan 0 Kernel-Level State Graph Implementation
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

#include "state_graph.h"
#include "pmm.h"
#include "kheap.h"
#include "vga.h"
#include "fs.h"
#include <stddef.h>
#include <stdint.h>
#include "kstring.h"

// Global state graph instance
static state_graph_t g_state_graph;

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
    g_state_graph.node_heap = (state_node_t*)kmalloc(g_state_graph.heap_size);
    g_state_graph.heap_used = 0;
    
    if (!g_state_graph.node_heap) {
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
    
    state_node_t* node = (state_node_t*)((uint8_t*)g_state_graph.node_heap + g_state_graph.heap_used);
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

/* ------------------------------------------------------------------ */
/* MFT Serialization Format (600-byte blob)                            */
/* Offset  Size  Field                                                 */
/* 0       8     id                                                    */
/* 8       4     type                                                  */
/* 12      4     flags                                                 */
/* 16      8     parent_id                                              */
/* 24      8     geometry.x                                             */
/* 32      8     geometry.y                                             */
/* 40      8     geometry.width                                         */
/* 48      8     geometry.height                                        */
/* 56      256   text[256]                                              */
/* 312     256   value[256]                                             */
/* 568     8     mft_record_id                                          */
/* 576     8     last_sync_hash                                         */
/* 584     16    reserved                                               */
/* ------------------------------------------------------------------ */
#define NODE_SERIAL_SIZE 600

static void node_serialize(state_node_t* node, uint8_t* buf) {
    uint64_t* qwords = (uint64_t*)buf;
    uint32_t* dwords = (uint32_t*)buf;

    qwords[0] = node->id;                          /* offset 0 */
    dwords[2] = (uint32_t)node->type;              /* offset 8 */
    dwords[3] = (uint32_t)node->flags;             /* offset 12 */
    qwords[2] = node->parent_id;                   /* offset 16 */
    qwords[3] = (int64_t)node->geometry.x;         /* offset 24 */
    qwords[4] = (int64_t)node->geometry.y;         /* offset 32 */
    qwords[5] = (int64_t)node->geometry.width;     /* offset 40 */
    qwords[6] = (int64_t)node->geometry.height;    /* offset 48 */
    /* text at offset 56 */
    for (int i = 0; i < 256; i++) {
        buf[56 + i] = (uint8_t)node->text[i];
    }
    /* value at offset 312 */
    for (int i = 0; i < 256; i++) {
        buf[312 + i] = (uint8_t)node->value[i];
    }
    qwords[71] = node->mft_record_id;              /* offset 568 */
    qwords[72] = node->last_sync_hash;             /* offset 576 */
    /* reserved at offset 584 -- zeroed by caller */
}

static void node_deserialize(const uint8_t* buf, state_node_t* node) {
    const uint64_t* qwords = (const uint64_t*)buf;
    const uint32_t* dwords = (const uint32_t*)buf;

    node->id = qwords[0];
    node->type = (node_type_t)dwords[2];
    node->flags = (node_flags_t)dwords[3];
    node->parent_id = qwords[2];
    node->geometry.x = (int)qwords[3];
    node->geometry.y = (int)qwords[4];
    node->geometry.width = (int)qwords[5];
    node->geometry.height = (int)qwords[6];
    for (int i = 0; i < 256; i++) {
        node->text[i] = (char)buf[56 + i];
    }
    for (int i = 0; i < 256; i++) {
        node->value[i] = (char)buf[312 + i];
    }
    node->mft_record_id = qwords[71];
    node->last_sync_hash = qwords[72];
}

/* ------------------------------------------------------------------ */
/* state_graph_mark_dirty -- mark a node dirty (needs MFT sync)        */
/* Already implemented above (line ~289).                              */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/* state_graph_sync_to_mft: write a single dirty node to the filesystem */
/* Returns 0 on success, -1 on failure.                               */
/* ------------------------------------------------------------------ */
int state_graph_sync_to_mft(node_id_t node_id) {
    state_node_t* node = state_graph_get_node(node_id);
    if (!node) return -1;

    /* Only sync dirty nodes */
    if (!(node->flags & NODE_FLAG_DIRTY)) {
        return 0;  /* nothing to sync */
    }

    /* Serialize the node */
    uint8_t buf[NODE_SERIAL_SIZE];
    for (int i = 0; i < NODE_SERIAL_SIZE; i++) buf[i] = 0;
    node_serialize(node, buf);

    /* Build filename: "sys/node_XXXXXXXX" */
    char path[64];
    path[0] = 's';
    path[1] = 'y';
    path[2] = 's';
    path[3] = '/';
    path[4] = 'n';
    path[5] = 'o';
    path[6] = 'd';
    path[7] = 'e';
    path[8] = '_';
    /* Convert node_id to 8 hex digits */
    const char hex[] = "0123456789ABCDEF";
    path[9]  = hex[(node_id >> 28) & 0xF];
    path[10] = hex[(node_id >> 24) & 0xF];
    path[11] = hex[(node_id >> 20) & 0xF];
    path[12] = hex[(node_id >> 16) & 0xF];
    path[13] = hex[(node_id >> 12) & 0xF];
    path[14] = hex[(node_id >> 8) & 0xF];
    path[15] = hex[(node_id >> 4) & 0xF];
    path[16] = hex[node_id & 0xF];
    path[17] = '\0';

    /* Write to filesystem */
    fs_file_t* file = fs_open(path, 1);  /* mode 1 = write */
    if (!file) {
        /* "sys" directory may not exist -- skip silently */
        return -1;
    }

    size_t written = fs_write(file, buf, NODE_SERIAL_SIZE);
    fs_close(file);

    if (written == NODE_SERIAL_SIZE) {
        /* Clear dirty flag */
        node->flags &= ~NODE_FLAG_DIRTY;
        node->last_sync_hash = 0;  /* simplified: always sync on next dirty */
        return 0;
    }

    return -1;
}

/* ------------------------------------------------------------------ */
/* state_graph_load_from_mft: load all persistent nodes from filesystem */
/* Runs after state_graph_init() during boot.                          */
/* Returns 0 on completion (even if no files found).                   */
/* ------------------------------------------------------------------ */
int state_graph_load_from_mft(void) {
    terminal_writestring("[STATE_GRAPH] Loading persistent nodes from MFT...\n");

    int loaded = 0;

    /* Scan node IDs 1..1024 for persistent nodes */
    for (node_id_t id = 1; id <= 1024; id++) {
        /* Build filename: "sys/node_XXXXXXXX" */
        char path[64];
        const char hex[] = "0123456789ABCDEF";
        path[0] = 's'; path[1] = 'y'; path[2] = 's'; path[3] = '/';
        path[4] = 'n'; path[5] = 'o'; path[6] = 'd'; path[7] = 'e'; path[8] = '_';
        path[9]  = hex[(id >> 28) & 0xF];
        path[10] = hex[(id >> 24) & 0xF];
        path[11] = hex[(id >> 20) & 0xF];
        path[12] = hex[(id >> 16) & 0xF];
        path[13] = hex[(id >> 12) & 0xF];
        path[14] = hex[(id >> 8) & 0xF];
        path[15] = hex[(id >> 4) & 0xF];
        path[16] = hex[id & 0xF];
        path[17] = '\0';

        /* Try to open the file (mode 0 = read) */
        fs_file_t* file = fs_open(path, 0);
        if (!file) {
            continue;  /* no persistent node for this ID */
        }

        /* Read serialized data */
        uint8_t buf[NODE_SERIAL_SIZE];
        size_t bytes_read = fs_read(file, buf, NODE_SERIAL_SIZE);
        fs_close(file);

        if (bytes_read < sizeof(node_id_t) + sizeof(int) + sizeof(int)) {
            continue;  /* malformed data */
        }

        /* Allocate a node from the dedicated heap */
        lock_acquire(&g_state_graph.lock);

        state_node_t* node = allocate_node();
        if (!node) {
            lock_release(&g_state_graph.lock);
            terminal_writestring("[STATE_GRAPH] Heap exhausted loading node\n");
            break;
        }

        /* Deserialize */
        node_deserialize(buf, node);

        /* Skip root-type nodes (recreated by init) */
        if (node->type >= NODE_TYPE_ROOT && node->type < NODE_TYPE_MAX && node->parent_id == NODE_ID_INVALID) {
            /* Root nodes are recreated in state_graph_init, skip restoring */
            g_state_graph.heap_used -= sizeof(state_node_t);  /* rollback */
            lock_release(&g_state_graph.lock);
            continue;
        }

        /* Adjust next_id if this node has a higher ID */
        if (node->id >= g_state_graph.next_id) {
            g_state_graph.next_id = node->id + 1;
        }

        /* Insert into hash map */
        hash_insert(node);

        /* Not dirty -- just loaded */
        node->flags &= ~NODE_FLAG_DIRTY;

        loaded++;
        lock_release(&g_state_graph.lock);
    }

    /* Rebuild parent-child relationships (second pass) */
    /* For each loaded node, find its parent and link as child */
    for (node_id_t id = 1; id <= 1024; id++) {
        state_node_t* child = state_graph_get_node(id);
        if (!child) continue;
        if (child->parent_id == NODE_ID_INVALID) continue;
        if (child->parent_id == child->id) continue;  /* self-reference guard */

        state_node_t* parent = state_graph_get_node(child->parent_id);
        if (!parent) {
            /* Parent not loaded -- make this a root, or skip */
            child->parent_id = NODE_ID_INVALID;
            continue;
        }

        /* Link child into parent's list (insert at head) */
        state_graph_add_child(child->parent_id, id);
    }

    terminal_writestring("[STATE_GRAPH] Loaded ");
    terminal_put_dec(loaded);
    terminal_writestring(" persistent node(s)\n");

    return 0;
}
