/*
 * openpaging.h - OpenSYS Paging Interface
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

#ifndef OPENPAGING_H
#define OPENPAGING_H

#include <stdint.h>
#include <stddef.h>

// Paging statistics
typedef struct {
    uint64_t total_pages;
    uint64_t free_pages;
    uint64_t kernel_heap_pages;
    uint64_t kernel_heap_used_pages;
    uint32_t page_size;
    uint8_t paging_level;
} openpaging_stats_t;

// Core API
void openpaging_init(void);
void openpaging_get_stats(openpaging_stats_t* stats);

// Page management
void* openalloc_pages(size_t pages);
void openfree_pages(void* ptr, size_t pages);

// Memory mapping
int openmap_physical(uint64_t physical, void* virtual, size_t size, uint32_t flags);
int openunmap_physical(void* virtual, size_t size);
void openpaging_map_huge(uint64_t vaddr, uint64_t paddr, uint64_t flags);

#endif // OPENPAGING_H
