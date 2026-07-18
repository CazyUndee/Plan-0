/*
 * ip.h - Minimal IP Layer Interface
 */

#ifndef IP_H
#define IP_H

#include <stdint.h>
#include "net.h"

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

#endif
