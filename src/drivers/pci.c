/*
 * pci.c - PCI Configuration Space Access
 *
 * PCI config space is accessed via I/O ports 0xCF8/0xCFC.
 * All 32-bit reads/writes must be to dword-aligned offsets.
 */

#include "pci.h"
#include "io.h"

/* Read a 32-bit value from PCI config space at the given bus/device/func/offset */
uint32_t pci_read_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t addr = pci_make_addr(bus, device, func, offset);
    outl(PCI_CONFIG_ADDRESS, addr);
    return inl(PCI_CONFIG_DATA);
}

/* Read a 16-bit value from PCI config space */
uint16_t pci_read_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t dword = pci_read_dword(bus, device, func, offset & 0xFC);
    return (uint16_t)((dword >> ((offset & 2) * 8)) & 0xFFFF);
}

/* Read an 8-bit value from PCI config space */
uint8_t pci_read_byte(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t dword = pci_read_dword(bus, device, func, offset & 0xFC);
    return (uint8_t)((dword >> ((offset & 3) * 8)) & 0xFF);
}

/* Write a 32-bit value to PCI config space */
void pci_write_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t addr = pci_make_addr(bus, device, func, offset);
    outl(PCI_CONFIG_ADDRESS, addr);
    outl(PCI_CONFIG_DATA, value);
}

/* Check if a device exists at the given address (vendor ID != 0xFFFF) */
int pci_device_exists(uint8_t bus, uint8_t device, uint8_t func) {
    uint16_t vendor = pci_read_word(bus, device, func, PCI_VENDOR_ID);
    return (vendor != 0xFFFF);
}

/* Scan PCI bus 0 for a device matching class/subclass/prog-if.
 * Returns 0 and fills out_bus/out_device/out_func on success, or -1 on failure. */
int pci_find_device(uint8_t class_code, uint8_t subclass, uint8_t prog_if,
                    uint8_t* out_bus, uint8_t* out_device, uint8_t* out_func) {
    for (int bus = 0; bus < 256; bus++) {
        for (int dev = 0; dev < 32; dev++) {
            for (int func = 0; func < 8; func++) {
                if (!pci_device_exists((uint8_t)bus, (uint8_t)dev, (uint8_t)func)) {
                    /* If function 0 doesn't exist, skip remaining functions */
                    if (func == 0) break;
                    continue;
                }

                uint8_t cls = pci_read_byte((uint8_t)bus, (uint8_t)dev, (uint8_t)func, PCI_CLASS);
                uint8_t sub = pci_read_byte((uint8_t)bus, (uint8_t)dev, (uint8_t)func, PCI_SUBCLASS);
                uint8_t prog = pci_read_byte((uint8_t)bus, (uint8_t)dev, (uint8_t)func, PCI_PROG_IF);

                if (cls == class_code && sub == subclass && prog == prog_if) {
                    if (out_bus) *out_bus = (uint8_t)bus;
                    if (out_device) *out_device = (uint8_t)dev;
                    if (out_func) *out_func = (uint8_t)func;
                    return 0;
                }

                /* If this is not a multi-function device, skip other functions */
                if (func == 0) {
                    uint8_t header_type = pci_read_byte((uint8_t)bus, (uint8_t)dev, 0, PCI_HEADER_TYPE);
                    if (!(header_type & 0x80)) break;
                }
            }
        }
    }
    return -1;  /* Not found */
}

/* Read a BAR and return its I/O base address (0 if not an I/O BAR) */
uint16_t pci_get_io_bar(uint8_t bus, uint8_t device, uint8_t func, int bar_index) {
    uint8_t offset = PCI_BAR0 + (bar_index * 4);
    uint32_t bar = pci_read_dword(bus, device, func, offset);

    /* Bit 0 = 0 means memory BAR, bit 0 = 1 means I/O BAR */
    if (!(bar & 0x01)) {
        return 0;  /* Not an I/O BAR */
    }

    /* For I/O BARs, bits 31-2 give the base address (I/O space is 16-bit) */
    return (uint16_t)(bar & 0xFFFC);
}
