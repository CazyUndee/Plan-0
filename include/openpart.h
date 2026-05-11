/*
 * openpart.h - OpenSYS Partition Management Interface
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

#ifndef OPENPART_H
#define OPENPART_H

#include <stdint.h>
#include "disk.h"

// Partition types
typedef enum {
    PARTITION_TYPE_UNUSED = 0,
    PARTITION_TYPE_EFI_SYSTEM = 1,
    PARTITION_TYPE_BIOS_BOOT = 2,
    PARTITION_TYPE_OPENFS_DATA = 3,
    PARTITION_TYPE_LINUX_SWAP = 4,
    PARTITION_TYPE_LINUX_DATA = 5
} partition_type_t;

// Partition information
typedef struct {
    int partition_number;
    partition_type_t type;
    uint64_t start_lba;
    uint64_t size_sectors;
    uint32_t filesystem_type;
    char label[37];  // GPT partition label
} openpart_info_t;

// Core API
void openpart_init(void);
int openpart_get_info(int partition_num, openpart_info_t* info);
int openpart_list_partitions(openpart_info_t* partitions, int max_count);

// Sector operations
int openpart_read_sectors(int partition_num, uint64_t start_lba, void* buffer, size_t sectors);
int openpart_write_sectors(int partition_num, uint64_t start_lba, const void* buffer, size_t sectors);

// Partition management
int openpart_format_partition(int partition_num, partition_type_t type);
int openpart_delete_partition(int partition_num);

// Disk information
int openpart_get_disk_info(opendisk_info_t* info);

#endif // OPENPART_H
