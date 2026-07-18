/*
 * idt64.c - 64-bit Interrupt Descriptor Table
 */

#include <stdint.h>
#include "idt.h"

#define IDT_ENTRIES 256

/* IDT entry (64-bit) */
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtr;

/* Interrupt gate type */
#define IDT_TYPE_INTERRUPT 0x8E
#define IDT_TYPE_TRAP      0x8F
#define IDT_TYPE_SYSCALL   0xEE  /* User-callable trap gate (DPL=3) */

/* Kernel code selector */
#define KERNEL_CS 0x08

static void idt_set_gate(int n, uint64_t handler) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].offset_mid = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].selector = KERNEL_CS;
    idt[n].ist = 0;
    idt[n].type_attr = IDT_TYPE_INTERRUPT;
    idt[n].reserved = 0;
}

static void idt_set_syscall_gate(int n, uint64_t handler) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].offset_mid = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].selector = KERNEL_CS;
    idt[n].ist = 0;
    idt[n].type_attr = IDT_TYPE_SYSCALL;
    idt[n].reserved = 0;
}

void idt_load(void) {
    __asm__ volatile ("lidt %0" : : "m"(idtr));
}

void idt_init(void) {
	idtr.limit = sizeof(idt) - 1;
	idtr.base = (uint64_t)&idt;

	for (int i = 0; i < IDT_ENTRIES; i++) {
		idt[i].offset_low = 0;
		idt[i].offset_mid = 0;
		idt[i].offset_high = 0;
		idt[i].selector = 0;
		idt[i].ist = 0;
		idt[i].type_attr = 0;
		idt[i].reserved = 0;
	}

	idt_set_gate(0, (uint64_t)isr0);
    idt_set_gate(1, (uint64_t)isr1);
    idt_set_gate(2, (uint64_t)isr2);
    idt_set_gate(3, (uint64_t)isr3);
    idt_set_gate(4, (uint64_t)isr4);
    idt_set_gate(5, (uint64_t)isr5);
    idt_set_gate(6, (uint64_t)isr6);
    idt_set_gate(7, (uint64_t)isr7);
    idt_set_gate(8, (uint64_t)isr8);
    idt_set_gate(9, (uint64_t)isr9);
    idt_set_gate(10, (uint64_t)isr10);
    idt_set_gate(11, (uint64_t)isr11);
    idt_set_gate(12, (uint64_t)isr12);
    idt_set_gate(13, (uint64_t)isr13);
    idt_set_gate(14, (uint64_t)isr14);
    idt_set_gate(15, (uint64_t)isr15);
    idt_set_gate(16, (uint64_t)isr16);
    idt_set_gate(17, (uint64_t)isr17);
    idt_set_gate(18, (uint64_t)isr18);
    idt_set_gate(19, (uint64_t)isr19);
    idt_set_gate(20, (uint64_t)isr20);
    idt_set_gate(21, (uint64_t)isr21);
    idt_set_gate(22, (uint64_t)isr22);
    idt_set_gate(23, (uint64_t)isr23);
    idt_set_gate(24, (uint64_t)isr24);
    idt_set_gate(25, (uint64_t)isr25);
    idt_set_gate(26, (uint64_t)isr26);
    idt_set_gate(27, (uint64_t)isr27);
    idt_set_gate(28, (uint64_t)isr28);
    idt_set_gate(29, (uint64_t)isr29);
    idt_set_gate(30, (uint64_t)isr30);
    idt_set_gate(31, (uint64_t)isr31);

    idt_set_gate(32, (uint64_t)irq0);
    idt_set_gate(33, (uint64_t)irq1);
    idt_set_gate(34, (uint64_t)irq2);
    idt_set_gate(35, (uint64_t)irq3);
    idt_set_gate(36, (uint64_t)irq4);
    idt_set_gate(37, (uint64_t)irq5);
    idt_set_gate(38, (uint64_t)irq6);
    idt_set_gate(39, (uint64_t)irq7);
    idt_set_gate(40, (uint64_t)irq8);
    idt_set_gate(41, (uint64_t)irq9);
    idt_set_gate(42, (uint64_t)irq10);
    idt_set_gate(43, (uint64_t)irq11);
    idt_set_gate(44, (uint64_t)irq12);
    idt_set_gate(45, (uint64_t)irq13);
    idt_set_gate(46, (uint64_t)irq14);
    idt_set_gate(47, (uint64_t)irq15);

    /* Syscall vector (0x80) */
    idt_set_syscall_gate(0x80, (uint64_t)syscall_entry);

    idt_load();
}

void idt_set_syscall_gate_wrapper(void) {
    idt_set_syscall_gate(0x80, (uint64_t)syscall_entry);
    idt_load();
}
