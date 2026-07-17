/*
 * pci.h - PCI Configuration Space Access
 *
 * Provides functions to read/write PCI configuration space
 * using I/O ports 0xCF8 (address) and 0xCFC (data).
 */

#ifndef PCI_H
#define PCI_H

#include <stdint.h>

/* PCI config address port and data port */
#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

/* PCI configuration space offsets (dword-aligned) */
#define PCI_VENDOR_ID       0x00
#define PCI_DEVICE_ID       0x02
#define PCI_COMMAND         0x04
#define PCI_STATUS          0x06
#define PCI_REVISION_ID     0x08
#define PCI_PROG_IF         0x09
#define PCI_SUBCLASS        0x0A
#define PCI_CLASS           0x0B
#define PCI_CLASS_CACHE     0x0C
#define PCI_HEADER_TYPE     0x0E
#define PCI_BAR0            0x10
#define PCI_BAR1            0x14
#define PCI_BAR2            0x18
#define PCI_BAR3            0x1C
#define PCI_BAR4            0x20
#define PCI_BAR5            0x24
#define PCI_SECONDARY_BUS   0x09

/* PCI command register bits */
#define PCI_CMD_IO_SPACE     0x0001
#define PCI_CMD_MEM_SPACE    0x0002
#define PCI_CMD_BUS_MASTER   0x0004

/* PCI class codes */
#define PCI_CLASS_SERIAL_BUS    0x0C
#define PCI_SUBCLASS_USB          0x03
#define PCI_PROGIF_UHCI           0x00
#define PCI_PROGIF_OHCI           0x10
#define PCI_PROGIF_EHCI           0x20
#define PCI_PROGIF_XHCI           0x30

/* Network */
#define PCI_CLASS_NETWORK         0x02
#define PCI_SUBCLASS_ETHERNET     0x00

/* PCI header types */
#define PCI_HEADER_TYPE_DEVICE  0x00
#define PCI_HEADER_TYPE_BRIDGE  0x01

/* Build PCI config address */
static inline uint32_t pci_make_addr(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    return (uint32_t)(0x80000000 | ((uint32_t)bus << 16) | ((uint32_t)device << 11) | ((uint32_t)func << 8) | (offset & 0xFC));
}

/* Read a 32-bit value from PCI config space */
uint32_t pci_read_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);

/* Read a 16-bit value from PCI config space */
uint16_t pci_read_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);

/* Read an 8-bit value from PCI config space */
uint8_t pci_read_byte(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);

/* Write a 32-bit value to PCI config space */
void pci_write_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t value);

/* Check if a device exists at the given address */
int pci_device_exists(uint8_t bus, uint8_t device, uint8_t func);

/* Find a PCI device by class/subclass/prog-if. Returns -1 if not found. */
int pci_find_device(uint8_t class_code, uint8_t subclass, uint8_t prog_if,
                    uint8_t* out_bus, uint8_t* out_device, uint8_t* out_func);

/* Get I/O base address from BAR (returns 0 if not I/O BAR, or the base address) */
uint16_t pci_get_io_bar(uint8_t bus, uint8_t device, uint8_t func, int bar_index);

#endif /* PCI_H */
