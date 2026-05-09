# OpenSYS OS v0.4.0

A pure 64-bit operating system built from scratch with modern Open* architecture.

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.

## Features

- **OpenKernel**: 64-bit kernel with direct long mode entry
- **OpenMemory**: PMM with bitmap allocator, 4-level paging, kernel heap
- **OpenFS**: NTFS-style filesystem with Master File Table (MFT) and attributes
- **OpenPart**: GUID Partition Table driver with ATA disk access
- **OpenInput**: USB HID keyboard driver framework
- **OpenShell**: Natural language commands (not cryptic Unix names)

## Project Structure

```
/
├── boot/
│   ├── boot.asm        # OpenKernel bootstrap
│   ├── interrupts.asm  # 64-bit interrupt stubs
│   ├── context_switch.asm # Task switching
│   ├── syscall.asm     # System call interface
│   └── usermode.asm    # User mode transitions
├── src/
│   ├── openkernel.c    # OpenKernel main
│   ├── pmm.c          # OpenMemory PMM
│   ├── paging.c       # 4-level paging
│   ├── kheap.c        # OpenMemory heap
│   ├── fs.c           # OpenFS implementation
│   ├── gpt.c          # OpenPart driver
│   ├── disk.c         # ATA disk driver
│   ├── hid_keyboard.c # OpenInput USB HID
│   └── shell.c        # OpenShell
├── include/
│   ├── stdint.h       # Standard integer types
│   ├── stddef.h       # Standard definitions
│   ├── pmm.h          # OpenMemory PMM interface
│   ├── paging.h       # Paging structures
│   ├── kheap.h        # OpenMemory heap
│   ├── fs.h           # OpenFS structures
│   ├── gpt.h          # OpenPart driver
│   ├── disk.h         # ATA driver
│   ├── usb.h          # USB host controller
│   ├── uhci.h         # UHCI driver
│   └── hid_keyboard.h # OpenInput USB HID
├── linker/
│   └── linker.ld      # OpenKernel linker script
├── grub.cfg           # GRUB configuration
├── Makefile           # Build system
└── .github/workflows/
    └── build.yml      # GitHub Actions CI
```

## Installation

### Quick Start (Pre-built Binary)

Download the latest release from GitHub:

1. Go to [OpenSYS Releases](https://github.com/CazyUndee/OpenSYS/releases)
2. Download the latest release (e.g., v0.4.0)
3. Choose either `os.iso` (bootable ISO) or `openkernel.bin` (raw kernel)

### Running with QEMU

**Option 1: Boot from ISO**
```bash
qemu-system-x86_64 -cdrom os.iso -nographic -m 128
```

**Option 2: Boot kernel directly**
```bash
qemu-system-x86_64 -kernel openkernel.bin -nographic -m 128
```

**Option 3: Boot with serial output**
```bash
qemu-system-x86_64 -kernel openkernel.bin -serial stdio -m 128
```

**Option 4: Boot with graphical output**
```bash
qemu-system-x86_64 -kernel openkernel.bin -m 128
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
2. **GRUB** -> Loads OpenKernel at 2MB, validates Multiboot2
3. **boot.asm** -> Direct 64-bit long mode entry
4. **openkernel.c** -> OpenMemory init, paging, heap, OpenFS, OpenShell

## Memory Layout

```
0x0000000000000000 - 0x00007FFFFFFFFFFF  User space (128TB)
0xFFFF800000000000 - 0xFFFF8000000FFFFF  Physical memory map
0xFFFF800000100000 - 0xFFFF800000FFFFFF  Kernel heap
0xFFFF800001000000 - ...                  Higher-half kernel code
```

## OpenFS Filesystem

NTFS-inspired design:
- Master File Table (MFT) with attribute records
- Supports: `$FILE_NAME`, `$DATA`, `$STANDARD_INFORMATION`
- File names as UTF-16LE (converted to ASCII)
- Resident data storage for small files
- **Non-resident data support for large files** (v0.4.0)
- **Attribute-based file metadata system**

## Changelog

### v0.4.0 (Current)
- **Complete Open* naming scheme** implemented
- **Removed all 32-bit code** - pure 64-bit architecture
- **Enhanced OpenFS** with non-resident data support
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

![Build](https://github.com/CazyUndee/OpenSYS/workflows/Build%20OpenSYS%20OS/badge.svg)

Automated builds via GitHub Actions. All commits are verified against:
- Project structure validation
- Open* naming consistency
- 64-bit build verification
- ISO creation and boot testing

## Next Steps

- [ ] USB controller initialization (UHCI/EHCI)
- [ ] More shell commands and aliases
- [ ] Real hardware testing
- [ ] Network stack implementation
- [ ] Graphics driver (VBE/EFI)

## License

Public domain.
