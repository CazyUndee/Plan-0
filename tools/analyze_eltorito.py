import os, struct
BASE = r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os'
GRUB = os.path.join(BASE, 'usr', 'lib', 'grub', 'i386-pc')

with open(os.path.join(GRUB, 'cdboot.img'), 'rb') as f:
    cdboot = f.read()
with open(os.path.join(GRUB, 'diskboot.img'), 'rb') as f:
    diskboot = f.read()
with open(os.path.join(GRUB, 'eltorito.img'), 'rb') as f:
    eltorito = f.read()

print('cdboot.img size:', len(cdboot))
print('diskboot.img size:', len(diskboot))
print('eltorito.img size:', len(eltorito))
print()
print('cdboot.img first 16:', cdboot[:16].hex())
print('diskboot.img first 16:', diskboot[:16].hex())
print('eltorito.img first 16:', eltorito[:16].hex())
print()

if eltorito[:16] == cdboot[:16]:
    print('eltorito starts like cdboot')
if eltorito[:16] == diskboot[:16]:
    print('eltorito starts like diskboot')
if eltorito[:512] == diskboot:
    print('eltorito starts with diskboot (512)')

# Check if kernel.img is embedded
kid = eltorito.find(kernel_img := open(os.path.join(GRUB, 'kernel.img'), 'rb').read())
if kid >= 0:
    print(f'kernel.img ({len(kernel_img)} bytes) found at offset {kid}')
else:
    print('kernel.img NOT found in eltorito.img')

# Check for various embedded module names
for s in [b'normal', b'biosdisk', b'iso9660', b'prefix', b'.mod']:
    idx = eltorito.find(s)
    if idx >= 0:
        ctx = eltorito[max(0,idx-5):idx+30]
        print(f'Found {s.decode()} at {hex(idx)}: {ctx[:50]}')
    else:
        print(f'{s.decode()} NOT found')

# Check diskboot-style sector total
print()
print('total_sectors at offset 4:', struct.unpack_from('<I', eltorito, 4)[0])

# Look at data after potential diskboot (first 512)
rest = eltorito[512:]
print(f'Data after first 512 bytes: {len(rest)} bytes')
print(f'First 16: {rest[:16].hex()}')

# Look for the "modules" section
# The kernel loads modules by scanning memory, they have a module header format
# Let's check for a module block by looking for known module names
for mod_name in [b'normal', b'biosdisk', b'iso9660', b'ext2', b'fat', b'part_msdos', b'configfile', b'fshelp']:
    idx = eltorito.find(mod_name)
    if idx >= 0:
        # Check if preceded by a length byte
        pre = eltorito[max(0,idx-2):idx]
        print(f'  Module name {mod_name.decode()} at {hex(idx)}, preceded by: {pre.hex()}')
