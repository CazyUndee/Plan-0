/*
 * openmemory.c - OpenSYS Memory Management
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

#include "../include/openmemory.h"
#include "../include/vga.h"
#include <stddef.h>
#include <stdint.h>

// Physical Memory Manager (PMM)
extern uint64_t pmm_get_total(void);
extern uint64_t pmm_get_free(void);
extern void pmm_init(uint64_t mbi);

// Kernel Heap Manager
extern void kheap_init(uint64_t start, uint64_t size);

// Initialize OpenMemory system
void openmemory_init(uint64_t mbi) {
    terminal_writestring("[OPENMEMORY] Initializing OpenMemory System...\n");
    
    // Initialize Physical Memory Manager
    pmm_init(mbi);
    terminal_writestring("  Total RAM: ");
    terminal_put_dec(pmm_get_total() / (1024 * 1024));
    terminal_writestring(" MB\n");
    terminal_writestring("  Free RAM: ");
    terminal_put_dec(pmm_get_free() / (1024 * 1024));
    terminal_writestring(" MB\n");
    
    // Initialize Kernel Heap
    kheap_init(0xFFFF800000000000ULL, 64 * 1024 * 1024);
    terminal_writestring("  Kernel Heap: 64MB at 0xFFFF800000000000\n");
    
    terminal_writestring("[OPENMEMORY] OpenMemory System Ready!\n");
}
