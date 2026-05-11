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

#include "openkheap.h"
#include "openmemory.h"
#include "vga.h"
#include <stddef.h>
#include <stdint.h>

typedef struct kheap_block {
    struct kheap_block* next;
    uint64_t size;
    uint16_t flags;
    uint16_t magic;
} kheap_block_t;

#define KHEAP_FREE      0x00
#define KHEAP_ALLOCATED 0x01
#define KHEAP_MAGIC     0xBEEF
#define MIN_BLOCK_SIZE  (sizeof(kheap_block_t) + 16)

#define ALIGN_UP(x, a)   (((x) + (a) - 1) & ~((a) - 1))
#define BLOCK_DATA(b)    ((void*)((uint64_t)(b) + sizeof(kheap_block_t)))
#define DATA_BLOCK(d)    ((kheap_block_t*)((uint64_t)(d) - sizeof(kheap_block_t)))

static kheap_block_t* heap_first = 0;
static uint64_t bytes_used = 0;
static uint64_t bytes_free = 0;

extern void* pmm_alloc_page(void);
extern void paging_map(uint64_t vaddr, uint64_t paddr, uint64_t flags);
#define PAGE_PRESENT  0x01
#define PAGE_WRITABLE 0x02

// Initialize OpenKHeap system
void openkheap_init(uint64_t start, uint64_t size) {
    terminal_writestring("[OPENKHEAP] Initializing Kernel Heap...\n");
    
    /* Allocate and map heap pages */
    for (uint64_t addr = start; addr < start + size; addr += 4096) {
        void* phys = pmm_alloc_page();
        if (phys) {
            paging_map(addr, (uint64_t)phys, PAGE_WRITABLE);
        }
    }
    
    heap_first = (kheap_block_t*)start;
    heap_first->next = 0;
    heap_first->size = size - sizeof(kheap_block_t);
    heap_first->flags = KHEAP_FREE;
    heap_first->magic = KHEAP_MAGIC;
    
    bytes_used = 0;
    bytes_free = size - sizeof(kheap_block_t);
    
    terminal_writestring("  Heap start: 0x");
    terminal_put_hex(start);
    terminal_writestring("\n");
    terminal_writestring("  Heap size: ");
    terminal_put_dec(size / (1024 * 1024));
    terminal_writestring(" MB\n");
    
    terminal_writestring("[OPENKHEAP] Kernel Heap Ready!\n");
}

static void split_block(kheap_block_t* block, uint64_t size) {
    uint64_t remaining = block->size - size - sizeof(kheap_block_t);
    if (remaining < MIN_BLOCK_SIZE) return;
    
    kheap_block_t* new_block = (kheap_block_t*)((uint64_t)block + sizeof(kheap_block_t) + size);
    new_block->next = block->next;
    new_block->size = remaining;
    new_block->flags = KHEAP_FREE;
    new_block->magic = KHEAP_MAGIC;
    
    block->size = size;
    block->next = new_block;
    
    bytes_free -= sizeof(kheap_block_t);
}

static void merge_blocks(void) {
    kheap_block_t* block = heap_first;
    while (block && block->next) {
        if (!(block->flags & KHEAP_ALLOCATED) && !(block->next->flags & KHEAP_ALLOCATED)) {
            uint64_t expected = (uint64_t)block + sizeof(kheap_block_t) + block->size;
            if ((uint64_t)block->next == expected) {
                kheap_block_t* next = block->next;
                block->size += sizeof(kheap_block_t) + next->size;
                block->next = next->next;
                bytes_free += sizeof(kheap_block_t);
                continue;
            }
        }
        block = block->next;
    }
}

// Allocate memory using OpenKHeap
void* openmalloc(size_t size) {
    if (size == 0) return 0;
    size = ALIGN_UP(size, 16);
    
    kheap_block_t* block = heap_first;
    while (block) {
        if (block->magic != KHEAP_MAGIC) return 0;
        if (!(block->flags & KHEAP_ALLOCATED) && block->size >= size) {
            split_block(block, size);
            block->flags = KHEAP_ALLOCATED;
            bytes_used += block->size;
            bytes_free -= block->size;
            return BLOCK_DATA(block);
        }
        block = block->next;
    }
    return 0;
}

// Free memory using OpenKHeap
void openfree(void* ptr) {
    if (!ptr) return;
    
    kheap_block_t* block = DATA_BLOCK(ptr);
    if (block->magic != KHEAP_MAGIC) return;
    if (!(block->flags & KHEAP_ALLOCATED)) return;
    
    block->flags = KHEAP_FREE;
    bytes_used -= block->size;
    bytes_free += block->size;
    
    merge_blocks();
}

void* openrealloc(void* ptr, size_t new_size) {
    if (new_size == 0) { openfree(ptr); return 0; }
    if (!ptr) return openmalloc(new_size);
    
    kheap_block_t* block = DATA_BLOCK(ptr);
    if (block->magic != KHEAP_MAGIC) return 0;
    if (new_size <= block->size) return ptr;
    
    void* new_ptr = openmalloc(new_size);
    if (!new_ptr) return 0;
    
    uint8_t* src = (uint8_t*)ptr;
    uint8_t* dst = (uint8_t*)new_ptr;
    for (uint64_t i = 0; i < block->size && i < new_size; i++) {
        dst[i] = src[i];
    }
    
    openfree(ptr);
    return new_ptr;
}

// Get heap statistics
void openkheap_get_stats(openkheap_stats_t* stats) {
    if (!stats) return;
    
    stats->total_size = bytes_used + bytes_free;
    stats->used_size = bytes_used;
    stats->free_blocks = 0;  // Would need counting logic
    stats->largest_free = 0;  // Would need scanning logic
}

// Legacy compatibility functions
void kheap_init(uint64_t start, uint64_t size) {
    openkheap_init(start, size);
}

void* kmalloc(size_t size) {
    return openmalloc(size);
}

void kfree(void* ptr) {
    openfree(ptr);
}

void* krealloc(void* ptr, size_t new_size) {
    return openrealloc(ptr, new_size);
}

uint64_t kheap_get_used(void) { return bytes_used; }
uint64_t kheap_get_free(void) { return bytes_free; }
