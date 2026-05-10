/*
 * mock_hardware.h - Mock Hardware Drivers for Testing
 *
 * Copyright (C) 2026 CazyUndee
 */

#ifndef MOCK_HARDWARE_H
#define MOCK_HARDWARE_H

#include <stdint.h>
#include <stddef.h>

// Mock disk driver
typedef struct {
    uint8_t* data;
    size_t size;
    size_t sector_size;
    uint32_t sector_count;
    int read_count;
    int write_count;
    int error_count;
} mock_disk_t;

// Mock VGA display
typedef struct {
    uint8_t* framebuffer;
    size_t width;
    size_t height;
    size_t cursor_x;
    size_t cursor_y;
    int puts_count;
    int clear_count;
} mock_vga_t;

// Mock timer
typedef struct {
    uint64_t current_time;
    uint64_t tick_interval;
    int tick_count;
    int callback_count;
} mock_timer_t;

// Mock PIC (Programmable Interrupt Controller)
typedef struct {
    uint8_t irq_mask;
    uint8_t irq_pending;
    uint8_t irq_service;
    int eoi_count;
    int mask_count;
} mock_pic_t;

// Mock serial port
typedef struct {
    uint8_t* buffer;
    size_t buffer_size;
    size_t read_pos;
    size_t write_pos;
    int read_count;
    int write_count;
} mock_serial_t;

// Mock RTC (Real Time Clock)
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    int read_count;
} mock_rtc_t;

// Mock disk functions
mock_disk_t* mock_disk_create(size_t size, size_t sector_size);
void mock_disk_destroy(mock_disk_t* disk);
int mock_disk_read(mock_disk_t* disk, uint32_t lba, uint32_t count, void* buffer);
int mock_disk_write(mock_disk_t* disk, uint32_t lba, uint32_t count, const void* buffer);
void mock_disk_inject_error(mock_disk_t* disk, int error_rate);

// Mock VGA functions
mock_vga_t* mock_vga_create(size_t width, size_t height);
void mock_vga_destroy(mock_vga_t* vga);
void mock_vga_puts(mock_vga_t* vga, const char* str);
void mock_vga_clear(mock_vga_t* vga);
void mock_vga_set_cursor(mock_vga_t* vga, size_t x, size_t y);

// Mock timer functions
mock_timer_t* mock_timer_create(uint64_t tick_interval);
void mock_timer_destroy(mock_timer_t* timer);
void mock_timer_tick(mock_timer_t* timer);
uint64_t mock_timer_get_time(mock_timer_t* timer);
void mock_timer_set_callback(mock_timer_t* timer, void (*callback)(void));

// Mock PIC functions
mock_pic_t* mock_pic_create(void);
void mock_pic_destroy(mock_pic_t* pic);
void mock_pic_mask_irq(mock_pic_t* pic, uint8_t irq);
void mock_pic_unmask_irq(mock_pic_t* pic, uint8_t irq);
void mock_pic_send_eoi(mock_pic_t* pic);
uint8_t mock_pic_get_irq(mock_pic_t* pic);

// Mock serial functions
mock_serial_t* mock_serial_create(size_t buffer_size);
void mock_serial_destroy(mock_serial_t* serial);
int mock_serial_read(mock_serial_t* serial, void* buffer, size_t size);
int mock_serial_write(mock_serial_t* serial, const void* buffer, size_t size);
void mock_serial_inject_data(mock_serial_t* serial, const void* data, size_t size);

// Mock RTC functions
mock_rtc_t* mock_rtc_create(uint8_t seconds, uint8_t minutes, uint8_t hours,
                           uint8_t day, uint8_t month, uint16_t year);
void mock_rtc_destroy(mock_rtc_t* rtc);
void mock_rtc_get_time(mock_rtc_t* rtc, uint8_t* seconds, uint8_t* minutes, 
                       uint8_t* hours, uint8_t* day, uint8_t* month, uint16_t* year);
void mock_rtc_set_time(mock_rtc_t* rtc, uint8_t seconds, uint8_t minutes, 
                       uint8_t hours, uint8_t day, uint8_t month, uint16_t year);

#endif // MOCK_HARDWARE_H
