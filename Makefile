# OpenSYS OS Makefile
# 64-bit only build

# Tools
CC = gcc
LD = ld
NASM = nasm

# Flags
CFLAGS = -m64 -ffreestanding -O0 -g -Wall -Wextra -fno-exceptions -nostdlib -fno-builtin -Iinclude -mcmodel=large
LDFLAGS = -m elf_x86_64 -T linker/linker.ld -nostdlib
NASMFLAGS = -f elf64

# Targets
KERNEL = openkernel
SRCS = openkernel.c pmm.c paging.c kheap.c fs.c gpt.c disk.c hid_keyboard.c shell.c vga.c io.c interrupt_handlers.c usb.c process.c scheduler.c vm.c elf.c syscall.c idt.c gdt.c tss.c user_bin.c pic.c timer.c ps2_keyboard.c ramfs.c switch_to.c isr.c rtc.c serial.c

# Directories
SRCDIR = src
BOOTDIR = boot
OBJDIR = obj
BINDIR = bin

# Files
BOOT_ASM = $(BOOTDIR)/boot.asm
SOURCES = $(SRCS:%=$(SRCDIR)/%)
OBJECTS = $(SRCS:%.c=$(OBJDIR)/%.o)
BOOT_OBJ = $(OBJDIR)/boot.o
ISR_OBJ = $(OBJDIR)/interrupts.o
CTXTSW_OBJ = $(OBJDIR)/context_switch.o
SYSCALL_OBJ = $(OBJDIR)/syscall_asm.o
EXTRA_ASM_OBJS = $(ISR_OBJ) $(CTXTSW_OBJ) $(SYSCALL_OBJ)

TARGET = $(BINDIR)/$(KERNEL).bin
ISO = $(BINDIR)/os.iso

# Phony targets
.PHONY: all clean run iso test check user_bin

# Default target
all: $(TARGET)

# User binary
user_bin: user/init.bin
	python3 tools/embed_binary.py user/init.bin src/user_bin

# Create directories
$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(BINDIR):
	@mkdir -p $(BINDIR)

# Boot assembly
$(BOOT_OBJ): $(BOOT_ASM) | $(OBJDIR)
	$(NASM) $(NASMFLAGS) -o $@ $<

# 64-bit assembly objects
$(ISR_OBJ): $(BOOTDIR)/interrupts.asm | $(OBJDIR)
	$(NASM) $(NASMFLAGS) -o $@ $<

$(CTXTSW_OBJ): $(BOOTDIR)/context_switch.asm | $(OBJDIR)
	$(NASM) $(NASMFLAGS) -o $@ $<

$(SYSCALL_OBJ): $(BOOTDIR)/syscall.asm | $(OBJDIR)
	$(NASM) $(NASMFLAGS) -o $@ $<

# C objects
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Link kernel
$(TARGET): $(BOOT_OBJ) $(EXTRA_ASM_OBJS) $(OBJECTS) | $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $(BOOT_OBJ) $(EXTRA_ASM_OBJS) $(OBJECTS)
	@echo "========================================"
	@echo "Build complete: $@"
	@echo "========================================"

# Create ISO
iso: $(TARGET)
	@mkdir -p iso/boot/grub
	@cp $(TARGET) iso/boot/kernel
	@cp grub.cfg iso/boot/grub/grub.cfg
	@grub-mkrescue -o $(ISO) iso
	@echo "ISO creation complete: $(ISO)"

# Run in QEMU
run: $(TARGET)
	@echo "Starting QEMU..."
	qemu-system-x86_64 -kernel $(TARGET) -serial stdio -m 128

run-iso: iso
	qemu-system-x86_64 -cdrom $(ISO) -serial stdio -m 128

# Test basic boot
test: $(TARGET)
	@echo "Running basic boot test..."
	@timeout 5s qemu-system-x86_64 -kernel $(TARGET) -display none -serial stdio -m 128 2>&1 | grep -q "OpenSYS" && echo "PASS" || echo "FAIL"

# Check dependencies
check:
	@echo "Checking build dependencies..."
	@which $(CC) >/dev/null 2>&1 || (echo "ERROR: $(CC) not found"; exit 1)
	@which $(LD) >/dev/null 2>&1 || (echo "ERROR: $(LD) not found"; exit 1)
	@which $(NASM) >/dev/null 2>&1 || (echo "ERROR: $(NASM) not found"; exit 1)
	@echo "All required tools found"

# User space build
user/init.o: user/init.c
	$(CC) -m64 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -mcmodel=small -c -o $@ $<

user/init.elf: user/init.o user/user.ld
	$(LD) -T user/user.ld -nostdlib -o $@ $<

user/init.bin: user/init.elf
	objcopy -O binary $< $@

# Clean build artifacts
clean:
	@rm -rf $(OBJDIR) $(BINDIR) iso
	@rm -f user/init.o user/init.elf user/init.bin
	@echo "Cleaned build artifacts"

# Show build configuration
info:
	@echo "OpenSYS OS Build Configuration (64-bit)"
	@echo "CC: $(CC)"
	@echo "LD: $(LD)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "Sources: $(SRCS)"
