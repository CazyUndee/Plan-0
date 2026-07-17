/*
 * icmp.c - Minimal ICMP Support (ping)
 */

#include <stdint.h>
#include "net.h"

int icmp_send_echo_request(ip_addr_t dst, uint16_t id, uint16_t seq) {
    (void)dst;(void)id;(void)seq;
    /* TODO: Build ICMP echo request packet and send via IP layer */
    return 0;
}
