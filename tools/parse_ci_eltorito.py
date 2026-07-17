import struct, os

BASE = r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os'

with open(os.path.join(BASE, 'bin', 'os-ci.iso'), 'rb') as f:
    iso_data = f.read()

CD = 2048

# Find El Torito Boot Record
for sector in range(16, 20):
    off = sector * CD
    desc_type = iso_data[off]
    if desc_type == 0:
        ident = iso_data[off+1:off+7]
        if ident == b'CD001\x20':
            boot_cat_lba = struct.unpack_from('<I', iso_data, off + 0x47)[0]
            print(f"Boot Record at sector {sector}, catalog LBA: {boot_cat_lba}")
            
            # Read boot catalog
            cat_off = boot_cat_lba * CD
            
            # Initial entry (at offset 32 from catalog start)
            init_entry = iso_data[cat_off+32:cat_off+64]
            print(f"Initial entry: {init_entry[:16].hex()}")
            
            if init_entry[0] == 0x88:
                boot_media = init_entry[1]
                load_seg = struct.unpack_from('<H', init_entry, 2)[0]
                sectors_to_load = struct.unpack_from('<H', init_entry, 6)[0]
                boot_lba = struct.unpack_from('<I', init_entry, 8)[0]
                print(f"  media={boot_media}, seg=0x{load_seg:X}")
                print(f"  sectors_to_load={sectors_to_load}, boot_lba={boot_lba}")
                
                # Read the boot file
                boot_offset = boot_lba * CD
                boot_size = sectors_to_load * CD
                boot_data = iso_data[boot_offset:boot_offset+boot_size]
                print(f"  boot file offset={boot_offset}, size={boot_size}")
                print(f"  first 16: {boot_data[:16].hex()}")
                
                # cdboot patches
                lba_val = struct.unpack_from('<I', boot_data, 0x0C)[0]
                sz_val = struct.unpack_from('<I', boot_data, 0x10)[0]
                print(f"  cdboot LBA patch: {lba_val}")
                print(f"  cdboot size patch: {sz_val}")
                
                if lba_val > 0:
                    core_offset = lba_val * CD
                    core_size = min(sz_val, len(iso_data) - core_offset)
                    core_data = iso_data[core_offset:core_offset+core_size]
                    print(f"  core at offset={core_offset}, size={len(core_data)}")
                    print(f"  core first 32: {core_data[:32].hex()}")
                    
                    # Save
                    out = os.path.join(BASE, 'bin', 'ci-core-data.bin')
                    with open(out, 'wb') as cf:
                        cf.write(core_data)
                    print(f"  Saved {out} ({len(core_data)} bytes)")
                    
                    # Analyze
                    grub = os.path.join(BASE, 'usr', 'lib', 'grub', 'i386-pc')
                    with open(os.path.join(grub, 'kernel.img'), 'rb') as kf:
                        kernel = kf.read()
                    
                    diskboot_offset = core_data.find(kernel[:32])
                    print(f"  kernel magic at offset: {diskboot_offset}")
                    
                    for name in [b'normal', b'biosdisk', b'iso9660', b'multiboot', b'elf', b'serial']:
                        idx = core_data.find(name)
                        if idx >= 0:
                            print(f"  Found {name.decode()} at offset {idx}")
                    
                    elf_count = 0
                    for i in range(len(core_data)-4):
                        if core_data[i:i+4] == b'\x7fELF':
                            elf_count += 1
                            i += 256
                    print(f"  ELF objects: {elf_count}")
