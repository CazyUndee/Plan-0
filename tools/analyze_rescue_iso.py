"""
Re-analyze the Debian GRUB rescue ISO to understand its boot structure.
We need the actual rescue ISO file.
"""
import os, struct, shutil

rescue_iso = r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os\bin\grub_rescue.iso'

# Check if we have the rescue ISO
if not os.path.exists(rescue_iso):
    print("Rescue ISO not found. Looking for it...")
    # Search common locations
    for root, dirs, files in os.walk(r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os'):
        for f in files:
            if 'rescue' in f.lower() and f.endswith('.iso'):
                print(f"  Found: {os.path.join(root, f)}")
    # Check download locations
    dl = r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads'
    for f in os.listdir(dl):
        if 'rescue' in f.lower() and f.endswith('.iso'):
            print(f"  Found: {os.path.join(dl, f)}")
else:
    print(f"Rescue ISO found: {rescue_iso}")
    size = os.path.getsize(rescue_iso)
    print(f"  Size: {size} bytes ({size // 2048} sectors of 2048)")

    # Read the first 64 sectors (System Area + Volume Descriptors)
    with open(rescue_iso, 'rb') as f:
        primary_vol = f.read(2048*64)
    
    # Check first 16 sectors (System Area)
    print(f"\nSystem Area (sectors 0-15, 32KB):")
    has_boot_code = False
    for sector in range(16):
        start = sector * 2048
        # Check for non-zero data
        data = primary_vol[start:start+2048]
        non_zero = sum(1 for b in data if b != 0)
        if non_zero > 0:
            print(f"  Sector {sector}: {non_zero} non-zero bytes")
            if sector == 0:
                # Check if it looks like cdboot.img
                if data[:4] == b'\xe8\x00\x00\xeb':
                    print(f"    -> First 4 bytes match cdboot.img!")
                    # Check for LBA and size
                    lba = struct.unpack_from('<I', data, 0x0C)[0]
                    size = struct.unpack_from('<I', data, 0x10)[0]
                    print(f"    -> cdboot.img LBA field at 0x0C: {lba} (0x{lba:x})")
                    print(f"    -> cdboot.img SIZE field at 0x10: {size} bytes ({size//2048} sectors)")
                    has_boot_code = True
            elif sector == 1:
                # Check for diskboot.img signature
                if data[:4] == b'\x52\x56\xbe\x1b':
                    print(f"    -> First 4 bytes match diskboot.img!")
                # Check for ELF magic (kernel.img)
                for off in range(0, 2048, 512):
                    if data[off:off+4] == b'\x7f\x45\x4c\x46':
                        e_type = struct.unpack_from('<H', data, off+16)[0]
                        e_machine = struct.unpack_from('<H', data, off+18)[0]
                        print(f"    -> ELF at offset {off}: e_type={e_type}, e_machine={e_machine}")
    
    if not has_boot_code:
        # Maybe the boot data is in LBA units (not sector 0)
        print("  No cdboot.img found in System Area")
    
    # Check the El Torito boot catalog
    # It's at sector 17 typically (right after Primary Volume Descriptor at sector 16)
    for sec in range(16, 32):
        start = sec * 2048
        data = primary_vol[start:start+2048]
        if data[:7] == b'\x00\x43\x44\x30\x30\x31\x01':  # El Torito "CD001" with boot indicator
            print(f"\nEl Torito Boot Catalog at sector {sec}:")
            # Validation entry (first 32 bytes)
            ve = data[0:32]
            print(f"  Validation entry: {ve[:32].hex()}")
            # Initial/default entry (second 32 bytes)
            ie = data[32:64]
            boot_indicator = ie[0]
            media_type = ie[1]
            boot_load_seg = struct.unpack_from('<H', ie, 2)[0]
            boot_sectors = ie[4]  # or from sector count field
            boot_lba = struct.unpack_from('<I', ie, 8)[0]
            print(f"  Boot entry: indicator={boot_indicator}, media={media_type}")
            print(f"  Load segment: 0x{boot_load_seg:x}")
            print(f"  Boot image starts at LBA: {boot_lba}")
            print(f"  File size (from IE): {((ie[4] << 0) | (ie[5] << 8) | (ie[6] << 16) | (ie[7] << 24))} sectors")
            # Actually let me re-read: boot_sectors is at byte offset 4 (2 bytes, little-endian)
            boot_sector_count = struct.unpack_from('<H', ie, 4)[0]
            print(f"  Boot sector count (from offset 4): {boot_sector_count}")
            print(f"  Boot image starts at LBA: {boot_lba}")
            
            # Read the boot image sectors
            if boot_lba > 0:
                with open(rescue_iso, 'rb') as f2:
                    f2.seek(boot_lba * 2048)
                    boot_img = f2.read(boot_sector_count * 512)  # For no-emulation, sector count is in 512-byte units
                    # Actually for no-emulation mode: sector count is in virtual 512-byte sectors
                    # Real sector size on CD is 2048 bytes
                    real_sectors = (boot_sector_count * 512 + 2047) // 2048
                    boot_img = f2.read(real_sectors * 2048)
                    print(f"\n  Boot image at LBA {boot_lba}:")
                    print(f"    El Torito sector count: {boot_sector_count} (512-byte virtual sectors)")
                    print(f"    Real sectors to read: {real_sectors}")
                    print(f"    Reading {len(boot_img)} bytes")
                    
                    if len(boot_img) >= 4:
                        print(f"    First 4 bytes: {boot_img[:4].hex()}")
                        if boot_img[:4] == b'\xe8\x00\x00\xeb':
                            print(f"    -> Matches cdboot.img!")
                            lba_val = struct.unpack_from('<I', boot_img, 0x0C)[0]
                            size_val = struct.unpack_from('<I', boot_img, 0x10)[0]
                            print(f"    -> cdboot.img LBA: {lba_val} (0x{lba_val:x})")
                            print(f"    -> cdboot.img SIZE: {size_val} bytes ({size_val//2048} sectors)")
                            
                            # Also check what's at that LBA
                            if lba_val > 0 and lba_val * 2048 + size_val <= size:
                                f2.seek(lba_val * 2048)
                                core_data = f2.read(size_val)
                                print(f"\n    Core data at LBA {lba_val}:")
                                print(f"      Size: {len(core_data)} bytes ({len(core_data)//2048} sectors)")
                                print(f"      First 4 bytes: {core_data[:4].hex()}")
                                if core_data[:2] == b'\x52\x56':
                                    print(f"      -> Matches diskboot.img!")
                                # Check for ELF magic
                                for off in range(0, min(len(core_data), 4096), 512):
                                    if core_data[off:off+4] == b'\x7f\x45\x4c\x46':
                                        print(f"      -> ELF at offset {off}")
                                        break
                            else:
                                print(f"    Core data LBA {lba_val} is outside ISO!")

print("\nDone.")
