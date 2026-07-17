/*
 * paging.h - Paging Interface
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

#ifndef PAGING_H
#define PAGING_H

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
} paging_stats_t;

// Core API
void paging_init(void);
void paging_get_stats(paging_stats_t* stats);

// Page management
void* alloc_pages(size_t pages);
void free_pages(void* ptr, size_t pages);

// Memory mapping
int map_physical(uint64_t physical, void* virtual, size_t size, uint32_t flags);
int unmap_physical(void* virtual, size_t size);
void paging_map_huge(uint64_t vaddr, uint64_t paddr, uint64_t flags);

#endif // PAGING_H
