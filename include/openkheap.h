/*
 * openkheap.h - OpenSYS Kernel Heap Interface
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

#ifndef OPENKHEAP_H
#define OPENKHEAP_H

#include <stdint.h>
#include <stddef.h>

// Heap statistics
typedef struct {
    uint64_t total_size;
    uint64_t used_size;
    uint64_t free_blocks;
    uint64_t largest_free;
    uint32_t block_count;
    uint32_t allocated_blocks;
} openkheap_stats_t;

// Core API
void openkheap_init(uint64_t start, uint64_t size);
void openkheap_get_stats(openkheap_stats_t* stats);

// Memory allocation
void* openmalloc(size_t size);
void openfree(void* ptr);

// Heap debugging
void openkheap_dump(void);
void openkheap_validate(void);

#endif // OPENKHEAP_H
