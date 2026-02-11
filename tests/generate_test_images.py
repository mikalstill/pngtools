#!/usr/bin/env python3
"""Generate additional test PNG images for pngtools testing.

Creates PNG files that exercise code paths not covered by the
existing sample images (paletted, interlaced, text chunks,
transparency).
"""

import os
import sys

from PIL import Image, PngImagePlugin


def generate_paletted(output_dir):
    """Create a 32x32 paletted PNG image."""
    img = Image.new('P', (32, 32))
    # Set a simple 4-colour palette
    palette = [0] * 768
    palette[0:3] = [255, 0, 0]      # index 0: red
    palette[3:6] = [0, 255, 0]      # index 1: green
    palette[6:9] = [0, 0, 255]      # index 2: blue
    palette[9:12] = [255, 255, 0]   # index 3: yellow
    img.putpalette(palette)

    # Fill with a pattern using all 4 colours
    pixels = img.load()
    for y in range(32):
        for x in range(32):
            pixels[x, y] = (x + y) % 4

    img.save(os.path.join(output_dir, 'paletted.png'))


def generate_interlaced(output_dir):
    """Create a 32x32 RGB PNG with Adam7 interlacing."""
    img = Image.new('RGB', (32, 32))
    pixels = img.load()
    for y in range(32):
        for x in range(32):
            pixels[x, y] = (x * 8, y * 8, 128)

    # Pillow does not support writing interlaced PNGs via
    # the save() method directly, so we use the low-level
    # PngImagePlugin writer.
    path = os.path.join(output_dir, 'interlaced.png')
    with open(path, 'wb') as f:
        writer = PngImagePlugin._save
        # PngImagePlugin._save signature:
        #   _save(im, fp, filename, chunk=putchunk,
        #          save_all=False)
        # We need to set the interlace flag. The simplest
        # way is to encode via the internal API.
        pass

    # Fallback: use the standard save and accept that Pillow
    # may not set interlace. We can use the raw approach
    # instead with zlib.
    _save_interlaced_png(img, path)


def _save_interlaced_png(img, path):
    """Save a PNG with Adam7 interlacing using raw bytes."""
    import struct
    import zlib

    width, height = img.size
    raw_data = img.tobytes()

    # Build IHDR
    ihdr_data = struct.pack(
        '>IIBBBBB',
        width, height,
        8,  # bit depth
        2,  # colour type (RGB)
        0,  # compression
        0,  # filter
        1   # interlace (Adam7)
    )

    # Build IDAT with filter byte (0 = None) per row
    raw_rows = b''
    stride = width * 3
    for y in range(height):
        raw_rows += b'\x00'  # filter byte
        raw_rows += raw_data[y * stride:(y + 1) * stride]

    compressed = zlib.compress(raw_rows)

    with open(path, 'wb') as f:
        # PNG signature
        f.write(b'\x89PNG\r\n\x1a\n')

        def write_chunk(chunk_type, data):
            f.write(struct.pack('>I', len(data)))
            f.write(chunk_type)
            f.write(data)
            crc = zlib.crc32(chunk_type + data) & 0xFFFFFFFF
            f.write(struct.pack('>I', crc))

        write_chunk(b'IHDR', ihdr_data)
        write_chunk(b'IDAT', compressed)
        write_chunk(b'IEND', b'')


def generate_with_text(output_dir):
    """Create a 32x32 RGB PNG with tEXt metadata chunks."""
    img = Image.new('RGB', (32, 32), color=(100, 150, 200))

    info = PngImagePlugin.PngInfo()
    info.add_text('Author', 'pngtools test suite')
    info.add_text('Description', 'Test image with text chunks')

    img.save(
        os.path.join(output_dir, 'with_text.png'),
        pnginfo=info
    )


def generate_with_transparency(output_dir):
    """Create a 32x32 paletted PNG with transparency."""
    img = Image.new('P', (32, 32))

    palette = [0] * 768
    palette[0:3] = [255, 0, 0]
    palette[3:6] = [0, 255, 0]
    palette[6:9] = [0, 0, 255]
    palette[9:12] = [255, 255, 0]
    img.putpalette(palette)

    pixels = img.load()
    for y in range(32):
        for x in range(32):
            pixels[x, y] = (x + y) % 4

    # Set transparency for palette index 0
    img.save(
        os.path.join(output_dir, 'with_transparency.png'),
        transparency=0
    )


def main():
    """Generate all test images."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    output_dir = os.path.join(project_root, 'testdata')

    os.makedirs(output_dir, exist_ok=True)

    generate_paletted(output_dir)
    generate_interlaced(output_dir)
    generate_with_text(output_dir)
    generate_with_transparency(output_dir)

    print(f'Generated test images in {output_dir}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
