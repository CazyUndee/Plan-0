/*
 * openmemory.c - OpenSYS Memory Management
 *
 * Copyright (C) 2026 CazyUndee
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under terms of the GNU General Public License as published by
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
#include "../include/pmm.h"
#include "../include/multiboot.h"
#include "../include/vga.h"
#include <stddef.h>
#include <stdint.h>

/* Bitmap: 1 bit per page */
static uint64_t* pmm_bitmap = 0;
static uint64_t pmm_bitmap_size = 0;
static uint64_t pmm_total_pages_count = 0;
static uint64_t pmm_free_pages_count = 0;

static uint32_t* pmm_ref_counts = 0;
static uint64_t pmm_ref_start = 0;
static uint64_t pmm_ref_end = 0;

extern uint64_t kernel_end;

static inline void bitmap_set(uint64_t index) {
    pmm_bitmap[index / 64] |= (1ULL << (index % 64));
}

static inline void bitmap_clear(uint64_t index) {
    pmm_bitmap[index / 64] &= ~(1ULL << (index % 64));
}

static inline int bitmap_test(uint64_t index) {
    return pmm_bitmap[index / 64] & (1ULL << (index % 64));
}

// Initialize OpenMemory system
void openmemory_init(uint64_t mbi) {
    terminal_writestring("[OPENMEMORY] Initializing OpenMemory System...\n");
    
    struct mboot_info* mbi_struct = (struct mboot_info*)mbi;

    /* Bitmap after kernel, aligned */
    uint64_t bitmap_start = (kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    pmm_bitmap = (uint64_t*)bitmap_start;

    uint64_t total_mem = 0;

    if (mbi_struct->flags & MBOOT_FLAG_MEM) {
        total_mem = ((uint64_t)mbi_struct->mem_upper + 1024) * 1024;
    }

    if (mbi_struct->flags & MBOOT_FLAG_MMAP) {
        struct mboot_mmap_entry* entry = (struct mboot_mmap_entry*)(uint64_t)mbi_struct->mmap_addr;
        struct mboot_mmap_entry* end = (struct mboot_mmap_entry*)((uint64_t)mbi_struct->mmap_addr + mbi_struct->mmap_length);

        uint64_t max_addr = 0;
        while ((uint8_t*)entry < (uint8_t*)end) {
            if (entry->type == MBOOT_MEM_AVAILABLE) {
                uint64_t base = ((uint64_t)entry->base_high << 32) | entry->base_low;
                uint64_t len = ((uint64_t)entry->length_high << 32) | entry->length_low;
                if (base + len > max_addr) max_addr = base + len;
            }
            entry = MMAP_ENTRY_NEXT(entry);
        }
        total_mem = max_addr;
    }

    pmm_total_pages_count = total_mem / PAGE_SIZE;
    pmm_bitmap_size = (pmm_total_pages_count + 63) / 64;

    /* Ref count array sits right after bitmap */
    uint64_t bitmap_bytes = pmm_bitmap_size * 8;
    uint64_t ref_start = bitmap_start + ((bitmap_bytes + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
    pmm_ref_counts = (uint32_t*)ref_start;
    pmm_ref_start = ref_start;
    pmm_ref_end = ref_start + ((pmm_total_pages_count * 4 + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));

    /* Mark all pages as used */
    for (uint64_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0xFFFFFFFFFFFFFFFFULL;
    }
    pmm_free_pages_count = 0;

    /* Zero ref counts */
    for (uint64_t i = 0; i < pmm_total_pages_count; i++) {
        pmm_ref_counts[i] = 0;
    }

    /* Free available pages from memory map */
    if (mbi_struct->flags & MBOOT_FLAG_MMAP) {
        struct mboot_mmap_entry* entry = (struct mboot_mmap_entry*)(uint64_t)mbi_struct->mmap_addr;
        struct mboot_mmap_entry* end = (struct mboot_mmap_entry*)((uint64_t)mbi_struct->mmap_addr + mbi_struct->mmap_length);

        while ((uint8_t*)entry < (uint8_t*)end) {
            if (entry->type == MBOOT_MEM_AVAILABLE) {
                uint64_t base = ((uint64_t)entry->base_high << 32) | entry->base_low;
                uint64_t len = ((uint64_t)entry->length_high << 32) | entry->length_low;
                openmemory_free_range(base, len);
            }
            entry = MMAP_ENTRY_NEXT(entry);
        }
    }
    
    terminal_writestring("  Total RAM: ");
    terminal_put_dec(pmm_total_pages_count * PAGE_SIZE / (1024 * 1024));
    terminal_writestring(" MB\n");
    terminal_writestring("  Free RAM: ");
    terminal_put_dec(pmm_free_pages_count * PAGE_SIZE / (1024 * 1024));
    terminal_writestring(" MB\n");
    
    terminal_writestring("[OPENMEMORY] OpenMemory System Ready!\n");
}

void* openmemory_alloc_page(void) {
    if (pmm_free_pages_count == 0) return 0;

    for (uint64_t i = 0; i < pmm_bitmap_size; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFFFFFFFFFFULL) {
            for (uint64_t j = 0; j < 64; j++) {
                if (!(pmm_bitmap[i] & (1ULL << j))) {
                    uint64_t page_index = i * 64 + j;
                    if (page_index >= pmm_total_pages_count) continue;

                    bitmap_set(page_index);
                    pmm_free_pages_count--;

                    return (void*)(page_index * PAGE_SIZE);
                }
            }
        }
    }
    return 0;
}

void openmemory_free_page(void* addr) {
    uint64_t page = (uint64_t)addr / PAGE_SIZE;
    if (page >= pmm_total_pages_count) return;
    if (!bitmap_test(page)) return;

    bitmap_clear(page);
    pmm_free_pages_count++;
}

void openmemory_reserve_range(uint64_t start, size_t length) {
    uint64_t first_page = start / PAGE_SIZE;
    uint64_t last_page = (start + length + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint64_t page = first_page; page < last_page && page < pmm_total_pages_count; page++) {
        if (!bitmap_test(page)) {
            bitmap_set(page);
            pmm_free_pages_count--;
        }
    }
}

void openmemory_free_range(uint64_t start, size_t length) {
    uint64_t first_page = (start + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t last_page = (start + length) / PAGE_SIZE;

    /* Skip first 1MB (reserved) */
    if (first_page < 256) first_page = 256;

    uint64_t bitmap_start = (uint64_t)pmm_bitmap;
    uint64_t bitmap_end = bitmap_start + pmm_bitmap_size * 8;

    for (uint64_t page = first_page; page < last_page && page < pmm_total_pages_count; page++) {
        uint64_t page_addr = page * PAGE_SIZE;

        /* Skip bitmap region */
        if (page_addr >= bitmap_start && page_addr < bitmap_end + PAGE_SIZE) continue;

        /* Skip ref-count array region */
        if (page_addr >= pmm_ref_start && page_addr < pmm_ref_end) continue;

        if (bitmap_test(page)) {
            bitmap_clear(page);
            pmm_free_pages_count++;
        }
    }
}

// Get memory statistics
void openmemory_get_stats(openmemory_stats_t* stats) {
    if (!stats) return;
    
    stats->total_mb = pmm_total_pages_count * PAGE_SIZE / (1024 * 1024);
    stats->free_mb = pmm_free_pages_count * PAGE_SIZE / (1024 * 1024);
    stats->used_mb = stats->total_mb - stats->free_mb;
    stats->heap_mb = 64;  // Fixed 64MB heap
}

// Legacy compatibility functions
void pmm_init(uint64_t mbi) {
    openmemory_init(mbi);
}

void* pmm_alloc_page(void) {
    return openmemory_alloc_page();
}

void pmm_free_page(void* addr) {
    openmemory_free_page(addr);
}

void pmm_reserve_range(uint64_t start, size_t length) {
    openmemory_reserve_range(start, length);
}

void pmm_free_range(uint64_t start, size_t length) {
    openmemory_free_range(start, length);
}

size_t pmm_get_total(void) { return pmm_total_pages_count * PAGE_SIZE; }
size_t pmm_get_free(void) { return pmm_free_pages_count * PAGE_SIZE; }
size_t pmm_get_total_pages(void) { return pmm_total_pages_count; }
size_t pmm_get_free_pages(void) { return pmm_free_pages_count; }
