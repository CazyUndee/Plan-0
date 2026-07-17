#include <stdint.h>
#include "rtc.h"
#include "io.h"

static int bcd_to_bin(int bcd) {
    return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

static uint8_t read_register(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

static void write_register(uint8_t reg, uint8_t val) {
    outb(0x70, reg);
    outb(0x71, val);
}

void rtc_init(void) {
    uint8_t reg_b = read_register(0x0B);
    reg_b |= 0x02;
    reg_b |= 0x04;
    reg_b &= ~0x80;
    write_register(0x0B, reg_b);
}

void rtc_read_time(rtc_time_t* time) {
    while (read_register(0x0A) & 0x80);

    time->second = read_register(0x00);
    time->minute = read_register(0x02);
    time->hour = read_register(0x04);
    time->day = read_register(0x07);
    time->month = read_register(0x08);
    time->year = read_register(0x09);
    time->century = read_register(0x32);

    uint8_t reg_b = read_register(0x0B);
    if (!(reg_b & 0x04)) {
        time->second = bcd_to_bin(time->second);
        time->minute = bcd_to_bin(time->minute);
        time->hour = bcd_to_bin(time->hour);
        time->day = bcd_to_bin(time->day);
        time->month = bcd_to_bin(time->month);
        time->year = bcd_to_bin(time->year);
        time->century = bcd_to_bin(time->century);
    }

    if (!(reg_b & 0x02) && (time->hour & 0x80)) {
        time->hour = ((time->hour & 0x7F) + 12) % 24;
    }
}
