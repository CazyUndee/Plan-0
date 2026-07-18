/*
 * icmp.c - Minimal ICMP Support (ping)
 */

#include <stdint.h>
#include "net.h"
#include "ip.h"

/* ICMP header */
typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
} icmp_header_t;

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO       8

static uint16_t icmp_checksum(const void* data, uint16_t len) {
    uint32_t sum = 0;
    const uint16_t* ptr = (const uint16_t*)data;
    for (int i = 0; i < len / 2; i++) {
        sum += ptr[i];
        while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    }
    if (len & 1) sum += ((const uint8_t*)data)[len - 1] << 8;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (~sum) & 0xFFFF;
}

int icmp_send_echo_request(ip_addr_t dst, uint16_t id, uint16_t seq) {
    (void)dst;(void)id;(void)seq;
    /* TODO: Build ICMP echo request packet and send via IP layer */
    return -1;
}

int icmp_handle_packet(const ipv4_header_t* ip, const void* data, uint16_t len) {
    (void)ip;
    if (len < sizeof(icmp_header_t)) return -1;
    const icmp_header_t* icmp = (const icmp_header_t*)data;
    if (icmp->type == ICMP_ECHO_REPLY) {
        /* Log ping reply (real handling would notify a waiting process) */
        return 0;
    }
    return -1;
}
