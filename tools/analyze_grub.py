"""Analyze GRUB boot images and figure out how to construct a valid one."""
import struct, os

base = r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os\usr\lib\grub\i386-pc'

# Read cdboot.img
with open(os.path.join(base, 'cdboot.img'), 'rb') as f:
    cdboot = f.read()

print(f"=== cdboot.img ===")
print(f"Size: {len(cdboot)} bytes")
print(f"First 128 bytes hex:")
for i in range(0, min(128, len(cdboot)), 16):
    hex_str = ' '.join(f'{b:02x}' for b in cdboot[i:i+16])
    print(f"  {i:04x}: {hex_str}")
print()

# Check for references (sector counts, LBA addresses) in cdboot.img
# Look for 4-byte values that could be LBA or sector count
print(f"Notable values at known offsets:")
# In GRUB's cdboot.S, the sector count for the core image is stored at a fixed offset
# Let's look at the last few bytes - often these contain the LBA address
print(f"Bytes 0x1F0-0x1FF: {' '.join(f'{b:02x}' for b in cdboot[0x1F0:])}")
print()

# Read diskboot.img
with open(os.path.join(base, 'diskboot.img'), 'rb') as f:
    diskboot = f.read()
print(f"=== diskboot.img ===")
print(f"Size: {len(diskboot)} bytes")
print(f"First 64 bytes hex:")
for i in range(0, min(64, len(diskboot)), 16):
    hex_str = ' '.join(f'{b:02x}' for b in diskboot[i:i+16])
    print(f"  {i:04x}: {hex_str}")

# diskboot.img first bytes should be a short jump (boot sector pattern)
print(f"First byte: {diskboot[0]:02x} (0xEB = short jmp, 0xE9 = near jmp, 0xEA = far jmp)")
print(f"Sector count at offset 2: {diskboot[2]} (0x{diskboot[2]:02x})")
print()

# Read boot_hybrid.img
with open(os.path.join(base, 'boot_hybrid.img'), 'rb') as f:
    hybrid = f.read()
print(f"=== boot_hybrid.img ===")
print(f"Size: {len(hybrid)} bytes")
print(f"MBR sig at 510: {hybrid[510]:02x}{hybrid[511]:02x}")
print(f"Partition table at 0x1BE:")
pe = hybrid[0x1BE:0x1CE]
print(f"  Entry 1: boot={pe[0]:02x} chs_start={pe[1]:02x}:{pe[2]:02x}:{pe[3]:02x} type={pe[4]:02x}")
print()

# Read boot.img
with open(os.path.join(base, 'boot.img'), 'rb') as f:
    boot = f.read()
print(f"=== boot.img ===")
print(f"Size: {len(boot)} bytes")
print(f"First 64 bytes:")
for i in range(0, min(64, len(boot)), 16):
    hex_str = ' '.join(f'{b:02x}' for b in boot[i:i+16])
    print(f"  {i:04x}: {hex_str}")

# Read kernel.img
with open(os.path.join(base, 'kernel.img'), 'rb') as f:
    kernel = f.read()
print(f"=== kernel.img ===")
print(f"Size: {len(kernel)} bytes")
print(f"ELF header: e_type={struct.unpack_from('<H', kernel, 16)[0]}, e_machine={struct.unpack_from('<H', kernel, 18)[0]}")
print(f"Entry point: 0x{struct.unpack_from('<Q' if kernel[4]==2 else '<I', kernel, 24)[0]:x}")

# Check what modules are embedded in kernel.img
# Look for ".mod" strings in kernel.img
mod_count = 0
pos = 0
while True:
    pos = kernel.find(b'.mod', pos)
    if pos == -1: break
    # Find the start of the filename
    start = pos
    while start > 0 and kernel[start-1:pos] not in [b'/', b'\\'] and kernel[start-1] != 0:
        start -= 1
    name = kernel[start:pos+4]
    try:
        name_str = name.decode('ascii', errors='replace')
        print(f"  Embedded module at offset {start}: {name_str}")
        mod_count += 1
    except:
        pass
    pos += 4
print(f"Total embedded module references: {mod_count}")
