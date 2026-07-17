import struct

# Read the cd-core.img
with open(r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os\bin\cd-core.img', 'rb') as f:
    data = f.read()

print(f'cd-core.img size: {len(data)} bytes')
print(f'First 64 bytes:')
print(' '.join(f'{b:02x}' for b in data[:64]))
print(f'MBR signature at 510: {data[510]:02x}{data[511]:02x}')

# Check against cdboot.img
with open(r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os\usr\lib\grub\i386-pc\cdboot.img', 'rb') as f:
    cdboot = f.read()
print(f'\ncdboot.img size: {len(cdboot)} bytes')
print(f'cd-core.img first {len(cdboot)} bytes match cdboot.img: {data[:len(cdboot)] == cdboot}')

# Check against diskboot.img
with open(r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os\usr\lib\grub\i386-pc\diskboot.img', 'rb') as f:
    diskboot = f.read()
print(f'diskboot.img size: {len(diskboot)} bytes')

# Check for kernel.img
with open(r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os\usr\lib\grub\i386-pc\kernel.img', 'rb') as f:
    kernel_img = f.read()
print(f'kernel.img size: {len(kernel_img)} bytes')
print(f'kernel.img first 16 bytes: {" ".join(f"{b:02x}" for b in kernel_img[:16])}')

# Check for ELF magic at various offsets
print('\nSearching for ELF magic in cd-core.img...')
for offset in range(0, len(data), 512):
    if data[offset:offset+4] == b'\x7fELF':
        print(f'  ELF magic at offset {offset}')
        e_type = struct.unpack_from('<H', data, offset+16)[0]
        e_machine = struct.unpack_from('<H', data, offset+18)[0]
        print(f'    e_type={e_type}, e_machine={e_machine}')

# Check if there's a GRUB kernel magic
# GRUB kernel.img also has a specific signature
grub_magic = b'\x67\x72\x75\x62'  # "grub" 
print(f'\nSearching for "grub" magic...')
for offset in range(0, len(data)):
    if data[offset:offset+4] == grub_magic:
        print(f'  Found at offset {offset}')
        print(f'  Context: {" ".join(f"{b:02x}" for b in data[max(0,offset-8):offset+16])}')
