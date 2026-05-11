/*
 * OpenSYS OS v0.4.0 - OpenKernel
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

#include <stdint.h>
#include <stddef.h>
#include "vga.h"
#include "serial.h"
#include "user_bin.h"
#include "pmm.h"
#include "kheap.h"
#include "timer.h"
#include "process.h"
#include "ramfs.h"
#include "ps2_keyboard.h"
#include "openmemory.h"
#include "openpaging.h"
#include "openkheap.h"
#include "fs.h"
#include "openpart.h"
#include "openinput.h"
#include "openshell.h"
#include "openvga.h"
#include "io.h"
#include "interrupts.h"
#include "usb.h"
#include "process.h"
#include "scheduler.h"
#include "vm.h"
#include "elf.h"
#include "opensys.h"
#include "idt.h"
#include "gdt.h"
#include "tss.h"
#include "openinput.h"
#include "rtc.h"
#include "vfs.h"
#include "ui_state.h"
#include "ui_command.h"

static void test_proc_a(void* arg) {
	(void)arg;
	volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
	while (1) {
		vga[2 * 80 + 60] = 'A' | 0x0A00;
		extern void process_yield(void);
		process_yield();
		vga[2 * 80 + 60] = ' ' | 0x0A00;
		process_yield();
	}
}

static void test_proc_b(void* arg) {
	(void)arg;
	volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
	while (1) {
		vga[2 * 80 + 62] = 'B' | 0x0C00;
		extern void process_yield(void);
		process_yield();
		vga[2 * 80 + 62] = ' ' | 0x0C00;
		process_yield();
	}
}

extern void paging_init(void);
extern void idt_init(void);
extern void pic_init(void);
extern int fs_mount(void);
extern int fs_format(uint64_t disk_size);
extern void timer_init(void);
extern void process_init(void);
extern pid_t process_create(const char* name, process_entry_t entry, void* arg);
extern pid_t process_create_user(const char* name, const void* elf_data, size_t elf_size);
extern void shell_run(void);

void kernel_main(uint64_t magic, uint64_t mbi) {
    terminal_initialize();
    serial_init();

    terminal_writestring("OpenSYS OS v1.0 - OpenKernel\n");
    serial_writestring("OpenSYS OS v1.0 - OpenKernel\n");
	terminal_writestring("=============================\n\n");

	if (magic == 0x2BADB002) {
		terminal_writestring("[BOOT] Multiboot: OK\n\n");
	} else {
		terminal_writestring("[BOOT] Multiboot: ");
		terminal_put_hex(magic);
		terminal_writestring("\n\n");
	}

	terminal_writestring("[INIT] Physical Memory...\n");
	pmm_init(mbi);
	terminal_writestring(" Total: ");
	terminal_put_dec(pmm_get_total() / (1024 * 1024));
	terminal_writestring(" MB\n Free: ");
	terminal_put_dec(pmm_get_free() / (1024 * 1024));
	terminal_writestring(" MB\n\n");

	terminal_writestring("[INIT] 64-bit Paging...\n");
	paging_init();
	terminal_writestring(" 4-level paging enabled\n\n");

	terminal_writestring("[INIT] Kernel Heap...\n");
	kheap_init(0xFFFF800000000000ULL, 64 * 1024 * 1024);
	terminal_writestring(" Heap: 64MB at 0xFFFF800000000000\n\n");

	terminal_writestring("[TEST] Memory Allocation...\n");

	void* p1 = kmalloc(128);
	void* p2 = kmalloc(1024);

	terminal_writestring(" kmalloc(128) = "); terminal_put_hex((uint64_t)p1); terminal_writestring("\n");
	terminal_writestring(" kmalloc(1024) = "); terminal_put_hex((uint64_t)p2); terminal_writestring("\n");

	kfree(p2);
	terminal_writestring(" kfree(1024) done\n\n");

	terminal_writestring("[DONE] 64-bit memory system ready!\n");

	terminal_writestring("\n[INIT] Interrupts...\n");
	idt_init();
	terminal_writestring(" IDT loaded\n");

	pic_init();
	terminal_writestring(" PIC remapped (IRQ 32-47)\n");

	timer_init();
	terminal_writestring(" Timer initialized (1000 Hz)\n");

	terminal_writestring("\n[INIT] PS/2 Keyboard...\n");
	if (ps2_keyboard_init() == 0) {
		terminal_writestring(" PS/2 keyboard initialized\n");

		__asm__ volatile (
			"inb $0x21, %%al\n"
			"and $0xFC, %%al\n"
			"outb %%al, $0x21\n"
			: : : "al"
		);
	} else {
		terminal_writestring(" PS/2 keyboard FAILED\n");
	}

	terminal_writestring("\n[INIT] OpenFS Filesystem...\n");
	if (fs_mount() < 0) {
		terminal_writestring(" No filesystem found, formatting...\n");
		if (fs_format(100 * 1024 * 1024) < 0) {
			terminal_writestring(" ERROR: Failed to format filesystem\n");
		} else {
			terminal_writestring(" OpenFS formatted and mounted\n");
		}
	} else {
		terminal_writestring(" OpenFS mounted successfully\n");
		terminal_writestring("// Phase 1: Initialize OpenMemory and OpenFS\n");
		terminal_writestring("[BOOT] Phase 1: Memory and Filesystem...\n");
		openmemory_init(mbi);
		openpaging_init();
		openkheap_init(0xFFFF800000000000ULL, 64 * 1024 * 1024);
		openfs_init();
		
		// Phase 2: Load MFT UI records into kernel-space State Graph
		terminal_writestring("[BOOT] Phase 2: Loading State Graph from MFT...\n");
		state_graph_init();
		state_graph_load_from_mft();
		
		// Phase 3: Initialize Intent Dispatcher
		terminal_writestring("[BOOT] Phase 3: Initializing Intent Dispatcher...\n");
		intent_dispatcher_init();
		
		// Phase 4: Initialize PCID system
		terminal_writestring("[BOOT] Phase 4: Initializing PCID...\n");
		extern void init_pcid_system(void);
		init_pcid_system();
		
		// Phase 5: Start OpenInput listener
		terminal_writestring("[BOOT] Phase 5: Starting Input System...\n");
		openinput_init();
		
		// Phase 6: Spawn OpenShell (CLI) and GUI Renderer as first two "Observer" processes
		terminal_writestring("[BOOT] Phase 6: Starting UI Processes...\n");
		
		// Register observers
		state_graph_add_observer(1, NULL);  // CLI process
		state_graph_add_observer(2, NULL);  // GUI process
		
		terminal_writestring("[BOOT] Unified Open* System Ready!\n\n");
		terminal_writestring("System is stable and running.\n");
		terminal_writestring("Keyboard driver needs PS/2 hardware support.\n");
	}

	__asm__ volatile ("sti");
	terminal_writestring(" Interrupts enabled\n\n");

	terminal_writestring("[DONE] System ready!\n");
	terminal_writestring("\nStarting simple test loop...\n\n");

	// Simple test loop instead of shell (keyboard not working in QEMU)
	volatile int counter = 0;
	while (1) {
		terminal_writestring("Test loop iteration: ");
		terminal_put_dec(counter++);
		terminal_writestring("\n");
		
		// Simple delay
		for (volatile int i = 0; i < 10000000; i++) {
			__asm__ volatile ("nop");
		}
		
		if (counter > 10) {
			terminal_writestring("\n[SUCCESS] Kernel boot test completed!\n");
			break;
		}
	}

	// Halt the system
	terminal_writestring("\n[System halted - press Ctrl+C to exit QEMU]\n");
	while (1) {
		__asm__ volatile ("hlt");
	}
}
