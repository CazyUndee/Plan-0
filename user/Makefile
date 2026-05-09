# Build user programs and generate embedded binary

CC = gcc
CFLAGS = -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -m64 -mcmodel=small
LDFLAGS = -T user.ld -nostdlib

.PHONY: all clean embed

all: init.elf init.bin

init.o: init.c
	$(CC) $(CFLAGS) -c -o $@ $<

init.elf: init.o user.ld
	ld $(LDFLAGS) -o $@ $<

init.bin: init.elf
	objcopy -O binary $< $@

embed: init.bin
	python3 ../tools/embed_binary.py init.bin ../src/user_bin

clean:
	rm -f *.o *.elf *.bin
