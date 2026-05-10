/*
 * openpaging.c - OpenSYS 64-bit Paging System
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

#include "../include/openpaging.h"
#include "../include/openmemory.h"
#include "../include/vga.h"
#include <stddef.h>
#include <stdint.h>

// External paging functions
extern void paging_init(void);

// Initialize OpenPaging system
void openpaging_init(void) {
    terminal_writestring("[OPENPAGING] Initializing 64-bit Paging...\n");
    
    // Initialize 4-level paging
    paging_init();
    
    terminal_writestring("  4-level paging enabled\n");
    terminal_writestring("  IA-32e long mode active\n");
    terminal_writestring("[OPENPAGING] OpenPaging System Ready!\n");
}

// Get paging statistics
void openpaging_get_stats(openpaging_stats_t* stats) {
    if (!stats) return;
    
    // Get memory stats
    openmemory_stats_t mem_stats;
    openmemory_get_stats(&mem_stats);
    
    // Calculate paging statistics (simplified)
    stats->total_pages = mem_stats.total_mb * 256;  // Approximate
    stats->free_pages = mem_stats.free_mb * 256;
    stats->kernel_heap_pages = mem_stats.kernel_heap_size / 4096;
    stats->kernel_heap_used_pages = mem_stats.kernel_heap_used / 4096;
}
