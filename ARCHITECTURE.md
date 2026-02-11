# pngtools Architecture

## Overview

pngtools is a suite of four command-line utilities for inspecting and
manipulating PNG image files. It was modelled on libtiff's tifftools
suite and is written in C against the libpng library. The project dates
from 2001 and uses GNU Autotools for its build system.

## Tools

### pnginfo (pnginfo.c, ~490 lines)

The primary tool. Reads PNG files via libpng and displays metadata:
image dimensions, bit depth, colour type, interlacing, compression,
resolution, and embedded text chunks. Has three modes:

- Default: display PNG metadata with descriptive labels
- `-t`: use tiffinfo-compatible label names
- `-d`: dump the image bitmap as hex pixel triples
- `-D`: verify bitmap extraction without displaying it

The bitmap dump includes run-length compression for zero-valued
pixels, printing them once and then showing a repeat count.

### pngchunks (pngchunks.c, ~180 lines)

Lists the raw chunk structure of a PNG file. Unlike pnginfo, this tool
does **not** use libpng -- it memory-maps the file with `mmap()` and
manually walks the PNG chunk linked list by parsing the 8-byte chunk
headers (4-byte length + 4-byte type). For IHDR chunks it also parses
and displays the IHDR payload fields. Each chunk's four-character name
is decoded to show its case-encoded properties (critical/ancillary,
public/private, etc.).

### pngchunkdesc (pngchunkdesc.c, ~48 lines)

A stdin/stdout filter that reads four-character PNG chunk names and
decodes the case information embedded in each letter. The same
`meanings` lookup table is duplicated between this file and
pngchunks.c.

### pngcp (pngcp.c + pngread.c + pngwrite.c + inflateraster.c, ~350 lines total)

Copies a PNG file while optionally changing bit depth (`-d`) and/or
the number of samples per pixel (`-s`). The pipeline is:

```
readimage() -> inflateraster() -> writeimage()
```

- **pngread.c / readimage()**: Opens the input PNG via libpng,
  expands palettes and sub-byte samples, returns the raw raster buffer
  along with width, height, bitdepth, and channel count.

- **inflateraster.c / inflateraster()**: Transforms the raster to the
  target bit depth and/or channel count using floating-point scale
  factors. Has known limitations with multi-byte samples and cannot
  perform both bitdepth and channel changes in a single pass.

- **pngwrite.c / writeimage()**: Writes the output PNG via libpng.
  Currently hardcodes `PNG_COLOR_TYPE_RGB` rather than deriving the
  colour type from the channel count.

## Header Files

- **pngcp.h**: Declares the three pngcp helper functions
  (`readimage`, `writeimage`, `inflateraster`). Note: the width/height
  parameter types here (`unsigned long *`) differ from the actual
  implementation in pngread.c which uses `png_uint_32 *`.

## Build System

GNU Autotools (autoconf + automake):

- **configure.ac**: Checks for a C compiler, libpng (`png_read_image`
  symbol), libm (`atan` symbol), and `png.h`.
- **Makefile.am**: Builds four binaries. pnginfo and pngchunks are
  standalone; pngcp links pngread.c, pngwrite.c, and inflateraster.c.
  pngcp additionally links libm.
- **man/**: DocBook SGML man page sources, built to man pages via
  `docbook2man`.

Build steps: `aclocal && autoconf && automake --add-missing &&
autoreconf && ./configure && make`

## CI

GitHub Actions (`.github/workflows/c.yml`): builds on Ubuntu with
libpng-dev and docbook-utils. Runs configure, make, and make install
to a staging directory. No automated tests are executed.

## Test Data

Five sample PNG files covering different configurations:

| File                      | Dimensions | Bit Depth | Colour Type |
|---------------------------|------------|-----------|-------------|
| sample.png                | 640x480    | 8         | RGB         |
| input.png                 | 256x256    | 8         | RGB         |
| foursamplesperpixel.png   | 32x32      | 8         | RGBA        |
| multibytesample.png       | 32x32      | 16        | Grayscale   |
| grayscale.png             | 32x32      | 4         | Grayscale   |

Additional generated test images in `testdata/` (created by
`tests/generate_test_images.py`):

| File                   | Dimensions | Bit Depth | Colour Type        |
|------------------------|------------|-----------|--------------------|
| paletted.png           | 32x32      | 8         | Paletted           |
| interlaced.png         | 32x32      | 8         | RGB (Adam7)        |
| with_text.png          | 32x32      | 8         | RGB + tEXt chunks  |
| with_transparency.png  | 32x32      | 8         | Paletted + tRNS    |

## Test Suite

48 automated tests using Python testtools + stestr, organised into
four test modules matching the four tools:

- `tests/test_pnginfo.py` -- metadata, tiff mode, bitmap dump, errors
- `tests/test_pngchunks.py` -- chunk listing, IHDR parsing, errors
- `tests/test_pngchunkdesc.py` -- case-bit decoding
- `tests/test_pngcp.py` -- copy, bitdepth/channel changes, errors

Tests run the compiled binaries via subprocess and assert on exit
codes and stdout/stderr content. See README.md for how to run them.

## Known Bugs and Issues

### Critical

1. **pnginfo.c:324 -- wrong array index in text chunk loop**: The
   switch reads `text[1].compression` instead of `text[i].compression`
   inside the `for (i = 0; i < num_text; i++)` loop. This means every
   text chunk displays the compression type of the *second* text chunk
   rather than its own.

2. **pnginfo.c:205-219 -- uninitialized variables**: `num_palette`
   and `num_trans` are declared but never assigned. They are used
   inside the `PNG_COLOR_TYPE_PALETTE` case to display palette and
   transparency counts, which means the output contains garbage values.
   These should be populated via `png_get_PLTE()` and
   `png_get_tRNS()`.

3. **pngwrite.c:67 -- hardcoded colour type**: `writeimage()` always
   sets `PNG_COLOR_TYPE_RGB` regardless of the actual number of
   channels. A comment in the code acknowledges this: "We need to
   derive a PNG color type from the number of channels and bitdepth".
   This means pngcp may produce invalid PNG files when the input is
   grayscale or has an alpha channel.

4. **pngcp.h type mismatch**: The header declares
   `readimage(..., unsigned long *width, unsigned long *height, ...)`
   but the implementation in pngread.c uses `png_uint_32 *` for both.
   On 64-bit systems where `unsigned long` is 8 bytes and
   `png_uint_32` is 4 bytes, this is a type mismatch that could cause
   memory corruption in pngcp.

### Moderate

5. **inflateraster.c limitations**: Two `todo_mikal` comments note
   that bitdepth scaling only works for single-byte depths (line 48)
   and that simultaneous bitdepth + channel changes fail (line 55).

6. **inflateraster.c:31 -- error sentinel**: On allocation failure,
   returns `(char *) -1` instead of NULL. The caller in pngcp.c checks
   for NULL, so this error path is never caught.

7. **pngchunks.c:163 -- cast through wrong type**: Casts the CRC
   offset to `long *` and dereferences it. Should use `int32_t *` or
   `uint32_t *` for portability and correctness.

8. **pngchunks.c:68 -- mmap return check**: Compares the `mmap()`
   return value with `< 0` but `mmap()` returns `MAP_FAILED`
   (typically `(void *) -1`), not a negative integer. The pointer
   comparison with `< 0` is undefined behaviour.

9. **Duplicated code**: The `meanings` lookup table for chunk name
   case decoding is defined identically in both pngchunkdesc.c and
   pngchunks.c.

### Minor

10. **pnginfo.c:160 -- fread return unchecked**: The return value of
    `fread()` when reading the PNG signature is not checked.

12. **Resource leaks on error**: pnginfo.c calls `pnginfo_error()`
    which exits immediately via `exit(1)`, leaking the open file
    handle and libpng structures. pngread.c has a goto-based cleanup
    pattern but it is incomplete (e.g. `raster` may be used
    uninitialized in the error path at line 92).

## Dependencies

- **libpng** (required): PNG reading/writing for pnginfo, pngcp
- **libm** (required): math functions for pngcp (pow in inflateraster)
- **docbook-utils** (optional): man page generation from SGML sources
