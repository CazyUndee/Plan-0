/*
 * ip.c - Minimal IP Layer
 */

#include <stdint.h>
#include "net.h"
#include "vga.h"

typedef struct __attribute__((packed)) {
    uint8_t  version_ihl;
    uint8_t  tos;
    uint16_t total_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t  ttl;
    uint8_t  proto;
    uint16_t checksum;
    uint32_t src;
    uint32_t dst;
} ipv4_header_t;

static uint16_t ip_checksum16(void* data, uint16_t len) {
    uint32_t sum = 0;
    uint16_t* ptr = (uint16_t*)data;
    for (int i = 0; i < len / 2; i++) {
        sum += ptr[i];
        while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    }
    if (len & 1) sum += ((uint8_t*)data)[len - 1] << 8;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (~sum) & 0xFFFF;
}

void net_handle_packet(const void* data, uint16_t len) {
    if (len < sizeof(ipv4_header_t)) return;
    const ipv4_header_t* ip = (const ipv4_header_t*)data;
    uint8_t version = (ip->version_ihl >> 4);
    if (version != 4) return;
    /* TODO: dispatch to ICMP, TCP, UDP */
}
