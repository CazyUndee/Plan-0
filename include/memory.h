/*
 * memory.h - Memory Management Interface
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

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

// Memory statistics
typedef struct {
    uint64_t total_mb;
    uint64_t free_mb;
    uint64_t kernel_heap_start;
    uint64_t kernel_heap_size;
    uint64_t kernel_heap_used;
} memory_stats_t;

// Core API
void memory_init(uint64_t mbi);
void memory_get_stats(memory_stats_t* stats);
void* memory_alloc_page(void);

// Memory allocation (for LibC)
void* memory_malloc(size_t size);
void memory_free(void* ptr);

// Memory information
uint64_t memory_get_total(void);
uint64_t memory_get_free(void);

// Memory range management
void memory_free_range(uint64_t start, size_t length);

#endif // MEMORY_H
