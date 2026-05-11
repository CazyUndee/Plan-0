/*
 * openmemory.h - OpenSYS Memory Management Interface
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

#ifndef OPENMEMORY_H
#define OPENMEMORY_H

#include <stdint.h>
#include <stddef.h>

// Memory statistics
typedef struct {
    uint64_t total_mb;
    uint64_t free_mb;
    uint64_t kernel_heap_start;
    uint64_t kernel_heap_size;
    uint64_t kernel_heap_used;
} openmemory_stats_t;

// Core API
void openmemory_init(uint64_t mbi);
void openmemory_get_stats(openmemory_stats_t* stats);
void* openmemory_alloc_page(void);

// Memory allocation (for OpenLibC)
void* openmalloc(size_t size);
void openfree(void* ptr);

// Memory information
uint64_t openmemory_get_total(void);
uint64_t openmemory_get_free(void);

// Memory range management
void openmemory_free_range(uint64_t start, size_t length);

#endif // OPENMEMORY_H
