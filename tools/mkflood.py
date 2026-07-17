import struct, os

proj = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Create MBR that writes 5000 bytes to debug port 0xE9
mbr = bytearray(512)

# mov cx, 5000 (0x1388)
# mov dx, 0x00E9
# loop:
#   mov al, 'A'
#   out dx, al
#   loop loop
# hlt
# jmp $-1
code = bytes([
    0xB9, 0x88, 0x13,  # mov cx, 5000
    0xBA, 0xE9, 0x00,  # mov dx, 0x00E9
    0xB0, 0x41,        # mov al, 'A'
    0xEE,              # out dx, al
    0xE2, 0xFC,        # loop -2
    0xF4,              # hlt
    0xEB, 0xFD,        # jmp $-1
])
mbr[0:len(code)] = code

# Partition table
pt = struct.pack('<B', 0x80) + b'\x01\x01\x00\x06\xFE\xFF\xFF'
pt += struct.pack('<II', 1, 131071)
mbr[446:462] = pt
mbr[510:512] = b'\x55\xAA'

img_path = os.path.join(proj, 'bin', 'flood.img')
with open(img_path, 'wb') as f:
    f.write(mbr)
    f.write(b'\x00' * (67108864 - 512))
print(f'Created {img_path} ({os.path.getsize(img_path)} bytes)')
