/*
 * ehci.h - EHCI (Enhanced Host Controller Interface) - USB 2.0
 *
 * High-speed (480 Mbps) USB host controller.
 */

#ifndef EHCI_H
#define EHCI_H

#include <stdint.h>
#include "pci.h"

/* EHCI MMIO registers (offsets from BAR0) */
#define EHCI_HCCAPBASE        0x00
#define EHCI_HCSPARAMS        0x04
#define EHCI_HCCPARAMS        0x08
#define EHCI_USBCMD           0x10
#define EHCI_USBSTS           0x14
#define EHCI_USBINTR          0x18
#define EHCI_FRINDEX          0x1C
#define EHCI_CTRLDSSEGMENT    0x20
#define EHCI_PERIODICLISTBASE 0x24
#define EHCI_ASYNCLISTADDR    0x28
#define EHCI_PORTSC           0x30

/* USBCMD bits */
#define EHCI_CMD_RS           (1U << 0)
#define EHCI_CMD_HCRESET      (1U << 1)
#define EHCI_CMD_FRLISTSIZE0  (1U << 2)
#define EHCI_CMD_FRLISTSIZE1  (1U << 3)
#define EHCI_CMD_PSEE         (1U << 4)
#define EHCI_CMD_ASE          (1U << 5)
#define EHCI_CMD_IAAD         (1U << 6)

/* USBSTS bits */
#define EHCI_STS_USBINT       (1U << 0)
#define EHCI_STS_USBERRINT    (1U << 1)
#define EHCI_STS_PCD          (1U << 2)
#define EHCI_STS_FLR          (1U << 3)
#define EHCI_STS_HSE          (1U << 4)
#define EHCI_STS_IAA          (1U << 5)
#define EHCI_STS_HC_HALTED    (1U << 12)

/* USBINTR bits */
#define EHCI_INTR_USBINT  (1U << 0)
#define EHCI_INTR_USBERR  (1U << 1)
#define EHCI_INTR_PCD     (1U << 2)
#define EHCI_INTR_FLR     (1U << 3)
#define EHCI_INTR_HSE     (1U << 4)
#define EHCI_INTR_IAA     (1U << 5)

/* PORTSC bits */
#define EHCI_PORTSC_CCS           (1U << 0)
#define EHCI_PORTSC_CSC             (1U << 1)
#define EHCI_PORTSC_PE            (1U << 2)
#define EHCI_PORTSC_PEC           (1U << 3)
#define EHCI_PORTSC_OC            (1U << 4)
#define EHCI_PORTSC_OCC           (1U << 5)
#define EHCI_PORTSC_FPR           (1U << 6)
#define EHCI_PORTSC_SUSP          (1U << 7)
#define EHCI_PORTSC_PR            (1U << 8)
#define EHCI_PORTSC_PP            (1U << 12)
#define EHCI_PORTSC_PO            (1U << 13)
#define EHCI_PORTSC_PT            (1U << 16)

typedef struct {
    volatile uint32_t* base;
    uint32_t           hcs_params;
    uint32_t           hcc_params;
    uint8_t            num_ports;
    uint8_t            caplen;
} ehci_controller_t;

/* Initialize EHCI */
int ehci_init(void);
/* Check if a device is connected to a port */
int ehci_port_connected(int port);
/* Reset an EHCI port */
int ehci_reset_port(int port);
/* Get EHCI port status */
uint32_t ehci_port_status(int port);

#endif /* EHCI_H */
