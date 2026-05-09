/*
 * serial.h - Serial Port Driver
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <stddef.h>

void serial_init(void);
void serial_putchar(char c);
void serial_write(const char* data, size_t size);
void serial_writestring(const char* data);
int serial_received(void);
char serial_getchar(void);

#endif
