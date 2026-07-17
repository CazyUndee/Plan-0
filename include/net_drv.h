/*
 * net_drv.h - Network Driver Interface
 *
 * Interface between the OS network stack and the specific card driver.
 */

#ifndef NET_DRV_H
#define NET_DRV_H

#include "net.h"
#include <stdint.h>

int net_drv_init(void);
int net_drv_send(const void* data, uint16_t len);
int net_drv_recv(void* buf, uint16_t len);
void net_drv_get_mac(mac_addr_t* mac);

#endif
