import struct, os

BASE = r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os'

with open(os.path.join(BASE, 'bin', 'os-ci.iso'), 'rb') as f:
    iso_data = f.read()

CD = 2048
print(f"ISO size: {len(iso_data)} bytes ({len(iso_data)//CD} CD sectors)")
print()

# Print all volume descriptors
for sector in range(16, 30):
    off = sector * CD
    if off >= len(iso_data):
        break
    desc_type = iso_data[off]
    ident = iso_data[off+1:off+7]
    
    if desc_type == 255:
        print(f"Sector {sector}: Volume Descriptor Set Terminator")
        break
    elif ident[:5] == b'CD001':
        names = {1: "Primary Volume Descriptor", 2: "Supplementary Volume Descriptor", 0: "Boot Record"}
        desc_name = names.get(desc_type, f"Unknown ({desc_type})")
        print(f"Sector {sector}: {desc_name}")
        
        if desc_type == 0:
            # Boot Record
            boot_sys_id = iso_data[off+7:off+24].rstrip(b' ').decode('ascii', errors='replace')
            boot_cat_lba = struct.unpack_from('<I', iso_data, off+0x47)[0]
            print(f"  Boot system ID: '{boot_sys_id}'")
            print(f"  Boot catalog LBA: {boot_cat_lba}")
    else:
        print(f"Sector {sector}: type={desc_type}, ident={ident}")

print(f"\nPrimary volume descriptor at sector 16:")
pvd = iso_data[16*CD:17*CD]
vol_id = pvd[8:40].rstrip(b' ').decode('ascii', errors='replace')
print(f"  Volume ID: '{vol_id}'")

# Boot Record at sector 17
boot_rec = iso_data[17*CD:18*CD]
boot_cat_lba = struct.unpack_from('<I', boot_rec, 0x47)[0]
print(f"\nBoot catalog LBA: {boot_cat_lba}")

# Read boot catalog
cat_off = boot_cat_lba * CD
print(f"Boot catalog at byte offset {cat_off}")

# Validation entry
val = iso_data[cat_off:cat_off+32]
print(f"Validation entry: {val[:16].hex()}")

# Initial entry  
init = iso_data[cat_off+32:cat_off+64]
print(f"Initial entry: {init[:16].hex()}")

if init[0] == 0x88:
    boot_media = init[1]
    load_seg = struct.unpack_from('<H', init, 2)[0]
    img_type = init[4]
    sectors = struct.unpack_from('<H', init, 6)[0]
    boot_lba = struct.unpack_from('<I', init, 8)[0]
    print(f"  Bootable CD entry:")
    print(f"  media={boot_media}, seg=0x{load_seg:X}, type={img_type}")
    print(f"  sectors={sectors}, boot_lba={boot_lba}")
    
    # Read boot file
    boot_offset = boot_lba * CD
    boot_size = sectors * CD
    print(f"  Boot file at offset {boot_offset}, size {boot_size}")
    boot_data = iso_data[boot_offset:boot_offset+boot_size]
    print(f"  First 16: {boot_data[:16].hex()}")
    
    lba_val = struct.unpack_from('<I', boot_data, 0x0C)[0]
    sz_val = struct.unpack_from('<I', boot_data, 0x10)[0]
    print(f"  cdboot LBA patch: {lba_val}")
    print(f"  cdboot size patch: {sz_val}")
    
    if lba_val > 0:
        core_offset = lba_val * CD
        core_size = min(sz_val, len(iso_data) - core_offset)
        core_data = iso_data[core_offset:core_offset+core_size]
        print(f"  Core at offset {core_offset}, size {len(core_data)}")
        print(f"  Core first 32: {core_data[:32].hex()}")
        
        out = os.path.join(BASE, 'bin', 'ci-core-data.bin')
        with open(out, 'wb') as cf:
            cf.write(core_data)
        print(f"  Saved {out}")
        
        grub = os.path.join(BASE, 'usr', 'lib', 'grub', 'i386-pc')
        with open(os.path.join(grub, 'kernel.img'), 'rb') as kf:
            kernel = kf.read()
        
        print(f"  kernel at 512: {core_data[512:512+len(kernel)] == kernel}")
        
        for name in [b'normal', b'biosdisk', b'iso9660', b'multiboot', b'elf', b'serial']:
            idx = core_data.find(name)
            if idx >= 0:
                print(f"  Found '{name.decode()}' at {idx}")
        
        elf_count = sum(1 for i in range(len(core_data)-4) if core_data[i:i+4] == b'\x7fELF')
        print(f"  ELF objects: {elf_count}")
