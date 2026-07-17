/*
 * ehci.c - EHCI (USB 2.0) Host Controller Driver
 *
 * Minimal high-speed USB host controller implementation.
 */

#include <stdint.h>
#include "ehci.h"
#include "pci.h"
#include "io.h"
#include "vga.h"
#include "pmm.h"

static ehci_controller_t g_ehci = {0};

static inline uint32_t ehci_read32(uint32_t reg) {
    volatile uint32_t* op_base = (volatile uint32_t*)((uint64_t)g_ehci.base + g_ehci.caplen);
    return op_base[reg >> 2];
}

static inline void ehci_write32(uint32_t reg, uint32_t val) {
    volatile uint32_t* op_base = (volatile uint32_t*)((uint64_t)g_ehci.base + g_ehci.caplen);
    op_base[reg >> 2] = val;
}

static void ehci_wait_ms(int ms) {
    for (int i = 0; i < ms * 50000; i++) {
        __asm__ volatile("pause");
    }
}

/* Init EHCI */
int ehci_init(void) {
    uint8_t bus, dev, func;
    uint32_t bar0;

    terminal_writestring("[EHCI] Scanning PCI bus for EHCI controller...\n");
    if (pci_find_device(PCI_CLASS_SERIAL_BUS, PCI_SUBCLASS_USB, PCI_PROGIF_EHCI,
                        &bus, &dev, &func) != 0) {
        terminal_writestring("[EHCI] No EHCI controller found\n");
        return -1;
    }

    /* Read BAR0 */
    bar0 = pci_read_dword(bus, dev, func, PCI_BAR0);
    if (bar0 == 0xFFFFFFFF || bar0 == 0) {
        terminal_writestring("[EHCI] Invalid BAR0\n");
        return -1;
    }

    /* Enable Bus Master and Memory Space */
    uint16_t cmd = pci_read_word(bus, dev, func, PCI_COMMAND);
    cmd |= PCI_CMD_BUS_MASTER | PCI_CMD_MEM_SPACE;
    pci_write_dword(bus, dev, func, PCI_COMMAND, cmd);

    g_ehci.base      = (volatile uint32_t*)(uint64_t)(bar0 & ~0x0Full);
    uint32_t capreg  = g_ehci.base[0];
    g_ehci.caplen    = (uint8_t)(capreg & 0xFF);
    g_ehci.hcs_params = g_ehci.base[EHCI_HCSPARAMS >> 2];
    g_ehci.hcc_params = g_ehci.base[EHCI_HCCPARAMS >> 2];
    g_ehci.num_ports  = (uint8_t)(g_ehci.hcs_params & 0xF);

    terminal_writestring("[EHCI] Controller at ");
    terminal_put_hex((uint64_t)g_ehci.base);
    terminal_writestring(", CAPLEN ");
    terminal_put_dec((uint64_t)g_ehci.caplen);
    terminal_writestring(", Ports ");
    terminal_put_dec((uint64_t)g_ehci.num_ports);
    terminal_writestring("\n");

    /* Halt controller */
    ehci_write32(EHCI_USBCMD, 0);
    while (!(ehci_read32(EHCI_USBSTS) & EHCI_STS_HC_HALTED)) {
        __asm__ volatile("pause");
    }

    /* Reset */
    ehci_write32(EHCI_USBCMD, EHCI_CMD_HCRESET);
    while (ehci_read32(EHCI_USBCMD) & EHCI_CMD_HCRESET) {
        __asm__ volatile("pause");
    }
    terminal_writestring("[EHCI] Controller reset OK\n");

    /* Allocate periodic frame list */
    phys_addr_t periodic_phys = pmm_alloc_page();
    if (!periodic_phys) {
        terminal_writestring("[EHCI] Failed to allocate periodic frame list\n");
        return -1;
    }
    volatile uint32_t* periodic = (volatile uint32_t*)((uint64_t)periodic_phys);
    for (int i = 0; i < 1024; i++) {
        periodic[i] = 1;
    }

    ehci_write32(EHCI_PERIODICLISTBASE, (uint32_t)periodic_phys);
    ehci_write32(EHCI_FRINDEX, 0);

    /* Enable interrupts */
    ehci_write32(EHCI_USBINTR, EHCI_INTR_PCD | EHCI_INTR_USBINT | EHCI_INTR_USBERR);

    /* Run */
    ehci_write32(EHCI_USBCMD, EHCI_CMD_RS | EHCI_CMD_PSEE | EHCI_CMD_ASE);
    ehci_wait_ms(10);
    terminal_writestring("[EHCI] Controller running\n");
    return 0;
}

int ehci_port_connected(int port) {
    if (!g_ehci.num_ports || port < 0 || (uint32_t)port >= g_ehci.num_ports) return 0;
    uint32_t portsc = ehci_read32(EHCI_PORTSC + (port * 4));
    return (portsc & EHCI_PORTSC_CCS) != 0;
}

uint32_t ehci_port_status(int port) {
    if (!g_ehci.num_ports || port < 0 || (uint32_t)port >= g_ehci.num_ports) return 0;
    return ehci_read32(EHCI_PORTSC + (port * 4));
}

int ehci_reset_port(int port) {
    if (!g_ehci.num_ports || port < 0 || (uint32_t)port >= g_ehci.num_ports) return -1;
    uint32_t portsc = ehci_read32(EHCI_PORTSC + (port * 4));
    portsc |= EHCI_PORTSC_PR;
    ehci_write32(EHCI_PORTSC + (port * 4), portsc);
    ehci_wait_ms(20);
    while (ehci_read32(EHCI_PORTSC + (port * 4)) & EHCI_PORTSC_PR) {
        __asm__ volatile("pause");
    }
    return 0;
}
