import pycdlib, os, struct

BASE = r'C:\Users\roone.DESKTOP-QK3UG2M\Downloads\projects\os'
iso_path = os.path.join(BASE, 'bin', 'os-ci.iso')
GRUB = os.path.join(BASE, 'usr', 'lib', 'grub', 'i386-pc')

iso = pycdlib.PyCdlib()
iso.open(iso_path)

# Read eltorito.img from CI ISO
# Try different pycdlib methods
import io
try:
    # Method 1: get_and_write
    buf = io.BytesIO()
    iso.get_file_from_iso_fp(buf, iso_path='/BOOT/GRUB/I386_PC/ELTORITO.IMG;1')
    boot_img_data = buf.getvalue()
    buf.close()
except Exception as e1:
    print("Method 1 failed:", e1)
    try:
        # Method 2: open and read with different API
        with iso.open_file_from_iso(iso_path='/BOOT/GRUB/I386_PC/ELTORITO.IMG;1') as fd2:
            boot_img_data = fd2.read()
    except Exception as e2:
        print("Method 2 failed:", e2)
        # Method 3: read raw from ISO file
        import mmap
        with open(iso_path, 'rb') as f:
            # Parse ISO directly to find the file
            f.seek(0)
            iso_data = f.read()
        print("Falling back to direct ISO parsing")
        boot_img_data = None

print("ELTORITO.IMG from CI ISO:", len(boot_img_data), "bytes")
print("First 32 hex:", boot_img_data[:32].hex())

cdboot_lba = struct.unpack_from('<I', boot_img_data, 0x0C)[0]
cdboot_size = struct.unpack_from('<I', boot_img_data, 0x10)[0]
print("cdboot LBA offset:", cdboot_lba, "size:", cdboot_size)

# Check diskboot
with open(os.path.join(GRUB, 'diskboot.img'), 'rb') as f:
    diskboot = f.read()
print("First 512 match diskboot:", boot_img_data[:512] == diskboot)

# Check kernel
with open(os.path.join(GRUB, 'kernel.img'), 'rb') as f:
    kernel = f.read()
print("kernel match at offset 2048:", boot_img_data[2048:2048+len(kernel)] == kernel)

# Analyze structure
after_cdboot = boot_img_data[2048:]
print("Data after cdboot:", len(after_cdboot), "bytes")

# Search for module names
for name in [b'normal', b'biosdisk', b'iso9660', b'multiboot', b'serial', b'boot', b'configfile']:
    idx = after_cdboot.find(name)
    if idx >= 0:
        print("  Found", name.decode(), "at offset", idx, "in after_cdboot")

# Save
out_path = os.path.join(BASE, 'bin', 'ci-eltorito.img')
with open(out_path, 'wb') as f:
    f.write(boot_img_data)
print("Saved to", out_path, "(", len(boot_img_data), "bytes)")

iso.close()
