/*
 * ip.c - Minimal IP Layer
 */

#include <stdint.h>
#include "ip.h"
#include "net.h"
#include "net_drv.h"
#include "vga.h"

/* ICMP handler (declared in icmp.c) */
int icmp_handle_packet(const ipv4_header_t* ip, const void* data, uint16_t len);

uint16_t ip_checksum16(void* data, uint16_t len) {
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

/* net.h API: send a raw packet via the NIC driver */
int net_send_packet(const void* data, uint16_t len) {
    return net_drv_send(data, len);
}

/* net.h API: receive a raw packet from the NIC driver */
int net_recv_packet(void* buf, uint16_t len) {
    return net_drv_recv(buf, len);
}

/* net.h API: get local MAC address */
void net_get_mac(mac_addr_t* mac) {
    net_drv_get_mac(mac);
}

/* IP packet dispatch */
void net_handle_packet(const void* data, uint16_t len) {
    if (len < sizeof(ipv4_header_t)) return;
    const ipv4_header_t* ip = (const ipv4_header_t*)data;
    uint8_t version = (ip->version_ihl >> 4);
    if (version != 4) return;

    /* Dispatch to transport layer based on protocol */
    const uint8_t* payload = (const uint8_t*)data + ((ip->version_ihl & 0x0F) * 4);
    uint16_t payload_len = len - ((ip->version_ihl & 0x0F) * 4);

    switch (ip->proto) {
        case NET_PROTO_ICMP:
            icmp_handle_packet(ip, payload, payload_len);
            break;
        case NET_PROTO_TCP:
            /* TCP not yet implemented */
            break;
        case NET_PROTO_UDP:
            /* UDP not yet implemented */
            break;
        default:
            break;
    }
}
