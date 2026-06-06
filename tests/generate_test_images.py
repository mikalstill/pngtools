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
    """Save a 24-bit RGB PNG with valid Adam7 interlacing.

    Pillow's high-level save() does not honour interlace=1, so we
    emit the seven Adam7 passes manually.
    """
    import struct
    import zlib

    width, height = img.size
    pixels = img.tobytes()
    stride = width * 3

    # Adam7: (col_start, row_start, col_step, row_step) per pass.
    passes = [
        (0, 0, 8, 8),
        (4, 0, 8, 8),
        (0, 4, 4, 8),
        (2, 0, 4, 4),
        (0, 2, 2, 4),
        (1, 0, 2, 2),
        (0, 1, 1, 2),
    ]

    raw_stream = b''
    for col_start, row_start, col_step, row_step in passes:
        pass_w = (width - col_start + col_step - 1) // col_step
        pass_h = (height - row_start + row_step - 1) // row_step
        if pass_w <= 0 or pass_h <= 0:
            continue
        for y in range(row_start, height, row_step):
            row = b'\x00'  # filter byte: None
            for x in range(col_start, width, col_step):
                off = y * stride + x * 3
                row += pixels[off:off + 3]
            raw_stream += row

    ihdr_data = struct.pack(
        '>IIBBBBB',
        width, height,
        8,  # bit depth
        2,  # colour type (RGB)
        0,  # compression
        0,  # filter
        1   # interlace (Adam7)
    )
    compressed = zlib.compress(raw_stream)

    with open(path, 'wb') as f:
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


def generate_with_text_after_idat(output_dir):
    """Create a PNG with a tEXt chunk placed *after* IDAT.

    Regression coverage for
    https://bugs.launchpad.net/ubuntu/+source/pngtools/+bug/1989739
    The PNG spec permits text chunks either side of IDAT, and
    `exiftool` historically wrote them at the tail of the file.
    Earlier pnginfo called png_get_text before consuming IDAT,
    so post-IDAT text chunks were invisible.
    """
    import struct
    import zlib

    width, height = 8, 8
    img = Image.new('RGB', (width, height), color=(50, 100, 150))
    raw = img.tobytes()

    ihdr = struct.pack(
        '>IIBBBBB', width, height, 8, 2, 0, 0, 0
    )

    stride = width * 3
    rows = b''
    for y in range(height):
        rows += b'\x00' + raw[y * stride:(y + 1) * stride]
    idat = zlib.compress(rows)

    keyword = b'Description'
    value = b'Hello, world!'
    text_data = keyword + b'\x00' + value

    def chunk(ctype, data):
        crc = zlib.crc32(ctype + data) & 0xFFFFFFFF
        return (
            struct.pack('>I', len(data)) + ctype + data
            + struct.pack('>I', crc)
        )

    path = os.path.join(output_dir, 'text_after_idat.png')
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n')
        f.write(chunk(b'IHDR', ihdr))
        f.write(chunk(b'IDAT', idat))
        f.write(chunk(b'tEXt', text_data))
        f.write(chunk(b'IEND', b''))


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
    generate_with_text_after_idat(output_dir)
    generate_with_transparency(output_dir)

    print(f'Generated test images in {output_dir}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
