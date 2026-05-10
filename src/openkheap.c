/*
 * openkheap.c - OpenSYS Kernel Heap Manager
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

#include "../include/openkheap.h"
#include "../include/openmemory.h"
#include "../include/vga.h"
#include <stddef.h>
#include <stdint.h>

// External heap functions
extern void kheap_init(uint64_t start, uint64_t size);
extern void* kmalloc(size_t size);
extern void kfree(void* ptr);

// Initialize OpenKHeap system
void openkheap_init(uint64_t start, uint64_t size) {
    terminal_writestring("[OPENKHEAP] Initializing Kernel Heap...\n");
    
    // Initialize the kernel heap
    kheap_init(start, size);
    
    terminal_writestring("  Heap start: 0x");
    terminal_put_hex(start);
    terminal_writestring("\n");
    terminal_writestring("  Heap size: ");
    terminal_put_dec(size / (1024 * 1024));
    terminal_writestring(" MB\n");
    
    terminal_writestring("[OPENKHEAP] Kernel Heap Ready!\n");
}

// Allocate memory using OpenKHeap
void* openmalloc(size_t size) {
    return kmalloc(size);
}

// Free memory using OpenKHeap
void openfree(void* ptr) {
    kfree(ptr);
}

// Get heap statistics
void openkheap_get_stats(openkheap_stats_t* stats) {
    if (!stats) return;
    
    // This would need heap implementation details
    stats->total_size = 64 * 1024 * 1024;  // Default 64MB
    stats->used_size = 0;  // Would need tracking
    stats->free_blocks = 0;  // Would need tracking
    stats->largest_free = 0;  // Would need tracking
}
