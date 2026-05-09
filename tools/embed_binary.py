#!/usr/bin/env python3
"""
embed_binary.py - Embed a raw binary file into C source/header files.

Usage:
    python embed_binary.py <input.bin> <output_base>

Generates:
    <output_base>.h  - header with extern declaration and size macro
    <output_base>.c  - source with the actual byte array data

Example:
    python embed_binary.py user/init.bin src/user_bin
    -> writes src/user_bin.h and src/user_bin.c
"""

import sys
import os

def embed_binary(input_path, output_base):
    with open(input_path, 'rb') as f:
        data = f.read()

    size = len(data)
    var_name = os.path.basename(output_base)

    # Generate header
    header_path = output_base + '.h'
    guard = var_name.upper() + '_H'

    with open(header_path, 'w') as f:
        f.write('/*\n')
        f.write(' * {} - Embedded binary data\n'.format(os.path.basename(header_path)))
        f.write(' * Auto-generated from {}\n'.format(os.path.basename(input_path)))
        f.write(' */\n\n')
        f.write('#ifndef {}\n'.format(guard))
        f.write('#define {}\n\n'.format(guard))
        f.write('#include <stdint.h>\n\n')
        f.write('#define {}_SIZE {}\n\n'.format(var_name.upper(), size))
        f.write('extern const unsigned char {}_data[];\n\n'.format(var_name))
        f.write('#endif\n')

    # Generate source
    source_path = output_base + '.c'

    with open(source_path, 'w') as f:
        f.write('/*\n')
        f.write(' * {} - Embedded binary data\n'.format(os.path.basename(source_path)))
        f.write(' * Auto-generated from {}\n'.format(os.path.basename(input_path)))
        f.write(' */\n\n')
        f.write('#include <stdint.h>\n')
        f.write('#include "../include/{}.h"\n\n'.format(var_name))
        f.write('const unsigned char {}_data[] = {{\n'.format(var_name))

        for i in range(0, size, 16):
            chunk = data[i:i+16]
            hex_vals = ', '.join('0x{:02X}'.format(b) for b in chunk)
            f.write('    {}\n'.format(hex_vals))

        if size == 0:
            f.write('    0x00\n')

        f.write('};\n')

    print('Generated {} ({} bytes) and {} ({} bytes of data)'.format(
        header_path, size, source_path, size))

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('Usage: {} <input.bin> <output_base>'.format(sys.argv[0]))
        print('Example: {} user/init.bin src/user_bin'.format(sys.argv[0]))
        sys.exit(1)

    embed_binary(sys.argv[1], sys.argv[2])
