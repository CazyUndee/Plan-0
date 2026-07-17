/*
 * tcp.c - Minimal TCP Support
 */

#include <stdint.h>
#include "net.h"

int tcp_connect(ip_addr_t dst, uint16_t port) {
    (void)dst;(void)port;
    return -1;
}

int tcp_send(int sock, const void* data, uint16_t len) {
    (void)sock;(void)data;(void)len;
    return -1;
}

int tcp_recv(int sock, void* buf, uint16_t len) {
    (void)sock;(void)buf;(void)len;
    return -1;
}
