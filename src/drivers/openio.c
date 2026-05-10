/*
 * openio.c - OpenSYS I/O Port Operations
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

#include "../include/openio.h"
#include <stdint.h>

// Output a byte to I/O port
void openoutb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Input a byte from I/O port
uint8_t openinb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Output a word to I/O port
void openoutw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

// Input a word from I/O port
uint16_t openinw(uint16_t port) {
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Output a double word to I/O port
void openoutd(uint16_t port, uint32_t value) {
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Input a double word from I/O port
uint32_t openind(uint16_t port) {
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// I/O port delay
void openio_wait(void) {
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

// Legacy compatibility functions
void outb(uint16_t port, uint8_t value) {
    openoutb(port, value);
}

uint8_t inb(uint16_t port) {
    return openinb(port);
}

void outw(uint16_t port, uint16_t value) {
    openoutw(port, value);
}

uint16_t inw(uint16_t port) {
    return openinw(port);
}

void outd(uint16_t port, uint32_t value) {
    openoutd(port, value);
}

uint32_t ind(uint16_t port) {
    return openind(port);
}

void io_wait(void) {
    openio_wait();
}
