# Plan 0 v0.4.0

A pure 64-bit operating system built from scratch.

## License

This project is licensed under the GNU Affero General Public License v3.0. See the [LICENSE](LICENSE) file for details.

## Features

- **Kernel**: 64-bit kernel with direct long mode entry
- **Memory Manager**: PMM with bitmap allocator, 4-level paging, kernel heap
- **FS**: NTFS-style filesystem with Master File Table (MFT) and attributes
- **Partition Driver**: GUID Partition Table driver with ATA disk access
- **Input**: USB HID keyboard driver framework
- **Shell**: Natural language commands (not cryptic Unix names)

## Project Structure

```
/
â”œâ”€â”€ boot/
â”‚   â”œâ”€â”€ boot.asm        # Kernel bootstrap
â”‚   â”œâ”€â”€ interrupts.asm  # 64-bit interrupt stubs
â”‚   â”œâ”€â”€ context_switch.asm # Task switching
â”‚   â”œâ”€â”€ syscall.asm     # System call interface
â”‚   â””â”€â”€ usermode.asm    # User mode transitions
â”œâ”€â”€ src/
â”‚   â”œâ”€â”€ kernel.c    # Kernel main
â”‚   â”œâ”€â”€ pmm.c          # PMM
â”‚   â”œâ”€â”€ paging.c       # 4-level paging
â”‚   â”œâ”€â”€ kheap.c        # Kernel heap
â”‚   â”œâ”€â”€ fs.c           # Filesystem implementation
â”‚   â”œâ”€â”€ gpt.c          # Partition driver
â”‚   â”œâ”€â”€ disk.c         # ATA disk driver
â”‚   â”œâ”€â”€ hid_keyboard.c # Input USB HID
â”‚   â””â”€â”€ shell.c        # Shell
â”œâ”€â”€ include/
â”‚   â”œâ”€â”€ stdint.h       # Standard integer types
â”‚   â”œâ”€â”€ stddef.h       # Standard definitions
â”‚   â”œâ”€â”€ pmm.h          # PMM interface
â”‚   â”œâ”€â”€ paging.h       # Paging structures
â”‚   â”œâ”€â”€ kheap.h        # Kernel heap
â”‚   â”œâ”€â”€ fs.h           # Filesystem structures
â”‚   â”œâ”€â”€ gpt.h          # Partition driver
â”‚   â”œâ”€â”€ disk.h         # ATA driver
â”‚   â”œâ”€â”€ usb.h          # USB host controller
â”‚   â”œâ”€â”€ uhci.h         # UHCI driver
â”‚   â””â”€â”€ hid_keyboard.h # Input USB HID
â”œâ”€â”€ linker/
â”‚   â””â”€â”€ linker.ld      # Kernel linker script
â”œâ”€â”€ grub.cfg           # GRUB configuration
â”œâ”€â”€ Makefile           # Build system
â””â”€â”€ .github/workflows/
    â””â”€â”€ build.yml      # GitHub Actions CI
```

## Installation

### Quick Start (Pre-built Binary)

Download the latest release from GitHub:

1. Go to [Plan 0 Releases](https://github.com/CazyUndee/Plan0/releases)
2. Download the latest release (e.g., v0.4.0)
3. Choose either `os.iso` (bootable ISO) or `kernel.bin` (raw kernel)

### Running with QEMU

**Option 1: Boot from ISO**
```bash
qemu-system-x86_64 -cdrom os.iso -nographic -m 128
```

**Option 2: Boot kernel directly**
```bash
qemu-system-x86_64 -kernel kernel.bin -nographic -m 128
```

**Option 3: Boot with serial output**
```bash
qemu-system-x86_64 -kernel kernel.bin -serial stdio -m 128
```

**Option 4: Boot with graphical output**
```bash
qemu-system-x86_64 -kernel kernel.bin -m 128
```

### Running on Real Hardware

1. **USB Boot (Recommended)**
   - Download `os.iso` from the release
   - Use a tool like Rufus (Windows) or dd (Linux) to write the ISO to a USB drive
   - Boot from the USB drive

2. **CD/DVD Boot**
   - Burn `os.iso` to a CD/DVD
   - Boot from the optical drive

### Building from Source

See the [Build Instructions](#build-instructions) section below.

## Prerequisites

### Windows (MSYS2)

```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-binutils
pacman -S mingw-w64-x86_64-nasm
pacman -S qemu
```

Add to PATH:
- `C:\msys64\mingw64\bin`
- `C:\msys64\usr\bin`

### Linux

```bash
sudo apt-get install gcc nasm qemu-system-x86 grub-pc-bin xorriso
```

## Build Instructions

```bash
make check     # Verify tools
make all       # Build 64-bit kernel
make iso       # Create bootable ISO
make run-iso   # Run in QEMU
```

## Boot Process

1. **BIOS** -> GRUB bootloader
2. **GRUB** -> Loads kernel at 2MB, validates Multiboot1
3. **boot.asm** -> Direct 64-bit long mode entry
4. **kernel.c** -> Memory init, paging, heap, filesystem, shell

## Memory Layout

```
0x0000000000000000 - 0x00007FFFFFFFFFFF  User space (128TB)
0xFFFF800000000000 - 0xFFFF8000000FFFFF  Physical memory map
0xFFFF800000100000 - 0xFFFF800000FFFFFF  Kernel heap
0xFFFF800001000000 - ...                  Higher-half kernel code
```

## Filesystem

NTFS-inspired design:
- Master File Table (MFT) with attribute records
- Supports: `$FILE_NAME`, `$DATA`, `$STANDARD_INFORMATION`
- File names as UTF-16LE (converted to ASCII)
- Resident data storage for small files
- **Non-resident data support for large files** (v0.4.0)
- **Attribute-based file metadata system**

## Changelog

### v0.4.0 (Current)
- **Removed all 32-bit code** - pure 64-bit architecture
- **Enhanced filesystem** with non-resident data support
- **Fixed GitHub Actions CI/CD** workflow
- **Updated all file names** to remove 32-bit suffixes
- **Improved build system** for 64-bit only

## Shell Commands

Natural language commands:

| Command | Description |
|---------|-------------|
| `show files` | List files in current directory |
| `show file <name>` | Display file contents |
| `make folder <name>` | Create directory |
| `make file <name>` | Create empty file |
| `remove <name>` | Delete file/folder |
| `go to <path>` | Change directory |
| `help` | Show all commands |

## CI/CD

![Build](https://github.com/CazyUndee/Plan0/workflows/Build%20Plan%200/badge.svg)

Automated builds via GitHub Actions. All commits are verified against:
- Project structure validation
- Build verification
- ISO creation and boot testing

## Next Steps

- [ ] USB controller initialization (UHCI/EHCI)
- [ ] More shell commands and aliases
- [ ] Real hardware testing
- [ ] Network stack implementation
- [ ] Graphics driver (VBE/EFI)

## License

GNU Affero General Public License v3.0.


