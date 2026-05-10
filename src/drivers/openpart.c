/*
 * openpart.c - OpenSYS Partition Management (GPT)
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

#include "../include/openpart.h"
#include "../include/openmemory.h"
#include "../include/vga.h"
#include <stddef.h>
#include <stdint.h>

// External disk and GPT functions
extern int disk_init(void);
extern int disk_read(uint64_t lba, void* buffer, size_t sectors);
extern int disk_write(uint64_t lba, const void* buffer, size_t sectors);
extern int gpt_init(void);
extern int gpt_list_partitions(void);

// Initialize OpenPart system
void openpart_init(void) {
    terminal_writestring("[OPENPART] Initializing Partition Management...\n");
    
    // Initialize disk driver
    if (disk_init() < 0) {
        terminal_writestring("  ERROR: Failed to initialize disk\n");
        return;
    }
    
    // Initialize GPT
    if (gpt_init() < 0) {
        terminal_writestring("  ERROR: Failed to initialize GPT\n");
        return;
    }
    
    terminal_writestring("  Disk initialized\n");
    terminal_writestring("  GPT support enabled\n");
    
    // List partitions
    int partition_count = gpt_list_partitions();
    terminal_writestring("  Found ");
    terminal_put_dec(partition_count);
    terminal_writestring(" partitions\n");
    
    terminal_writestring("[OPENPART] Partition Management Ready!\n");
}

// Get partition information
int openpart_get_info(int partition_num, openpart_info_t* info) {
    if (!info || partition_num < 0) return -1;
    
    // This would need GPT implementation details
    info->partition_number = partition_num;
    info->type = 0;  // Would be determined from GPT
    info->start_lba = 0;
    info->size_sectors = 0;
    info->filesystem_type = 0;
    
    return 0;
}

// List all partitions
int openpart_list_partitions(openpart_info_t* partitions, int max_count) {
    if (!partitions || max_count <= 0) return -1;
    
    int count = gpt_list_partitions();
    if (count > max_count) count = max_count;
    
    for (int i = 0; i < count; i++) {
        openpart_get_info(i, &partitions[i]);
    }
    
    return count;
}

// Read partition sectors
int openpart_read_sectors(int partition_num, uint64_t start_lba, void* buffer, size_t sectors) {
    // This would need partition-specific LBA calculation
    return disk_read(start_lba, buffer, sectors);
}

// Write partition sectors
int openpart_write_sectors(int partition_num, uint64_t start_lba, const void* buffer, size_t sectors) {
    // This would need partition-specific LBA calculation
    return disk_write(start_lba, buffer, sectors);
}
