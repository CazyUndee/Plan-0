/*
 * net.h - Minimal Network Stack
 *
 * Basic IP/TCP/UDP skeleton for Plan 0.
 */

#ifndef NET_H
#define NET_H

#include <stdint.h>

typedef struct {
    uint8_t addr[6];
} mac_addr_t;

typedef struct {
    uint8_t addr[4];
} ip_addr_t;

#define NET_PROTO_ICMP 1
#define NET_PROTO_TCP  6
#define NET_PROTO_UDP  17

/* Initialize the network stack */
int net_init(void);
/* Send an Ethernet packet */
int net_send_packet(const void* data, uint16_t len);
/* Receive a packet into a buffer, returns length */
int net_recv_packet(void* buf, uint16_t len);
/* Get local MAC address */
void net_get_mac(mac_addr_t* mac);
/* Handle a received packet (protocol dispatch) */
void net_handle_packet(const void* data, uint16_t len);

#endif
