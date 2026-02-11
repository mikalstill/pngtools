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
decodes the case information embedded in each letter. Uses the shared
`chunk_meanings` lookup table from `chunk_meanings.h`.

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
  target bit depth and/or channel count. Processes all pixels in a
  single pass, handling both bitdepth scaling and channel mapping
  together. Supports multi-byte samples (e.g. 16-bit) using
  big-endian byte order. Channel mapping supports gray-to-RGB
  expansion, alpha addition/removal, and direct channel copying.

- **pngwrite.c / writeimage()**: Writes the output PNG via libpng.
  Derives the PNG colour type from the channel count (1=gray,
  2=gray+alpha, 3=RGB, 4=RGBA).

## Header Files

- **pngcp.h**: Declares the three pngcp helper functions
  (`readimage`, `writeimage`, `inflateraster`). Uses `png_uint_32`
  for width/height to match the libpng API.

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

## Code Quality

- **clang-format**: Enforces consistent formatting using the GNU
  base style. Configuration in `.clang-format`. Run
  `scripts/check-format.sh fix` to auto-format, or
  `scripts/check-format.sh` to check without modifying.

- **cppcheck**: Static analysis for warnings and style issues.
  Runs with `--enable=warning,style` in both CI and pre-commit.

- **shellcheck**: Lints shell scripts in `scripts/`.

- **actionlint**: Validates GitHub Actions workflow YAML files.

- **CodeQL**: GitHub's semantic code analysis for security
  vulnerabilities and code quality. Runs as a separate CI
  workflow (`.github/workflows/codeql.yml`) with the
  `security-and-quality` query suite. Also runs weekly on a
  schedule to catch newly discovered vulnerability patterns.

## CI

Two GitHub Actions workflows:

**CI** (`.github/workflows/c.yml`): runs actionlint, shellcheck,
clang-format, and cppcheck checks first, then builds on Ubuntu
with libpng-dev and docbook-utils. Runs the full autotools chain,
configure, make, and make install to a staging directory. Then
sets up a Python venv, installs test dependencies, generates test
images, and runs the full test suite via stestr.

**CodeQL** (`.github/workflows/codeql.yml`): runs on push, PR,
and weekly schedule. Performs deep semantic security and quality
analysis of the C source code.

Five pre-commit hooks run automatically before each commit:
1. **actionlint** -- validates workflow YAML
2. **shellcheck** -- lints shell scripts
3. **clang-format** -- checks source formatting
4. **cppcheck** -- static analysis
5. **build-and-test** -- full build and test cycle

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

53 automated tests using Python testtools + stestr, organised into
four test modules matching the four tools:

- `tests/test_pnginfo.py` -- metadata, tiff mode, bitmap dump, errors
- `tests/test_pngchunks.py` -- chunk listing, IHDR parsing, errors
- `tests/test_pngchunkdesc.py` -- case-bit decoding
- `tests/test_pngcp.py` -- copy, bitdepth/channel changes, errors

Tests run the compiled binaries via subprocess and assert on exit
codes and stdout/stderr content. See README.md for how to run them.

## Known Bugs and Issues

No known bugs.

## Dependencies

- **libpng** (required): PNG reading/writing for pnginfo, pngcp
- **libm** (required): math functions for pngcp (pow in inflateraster)
- **docbook-utils** (optional): man page generation from SGML sources
- **clang-format** (development): code formatting enforcement
- **cppcheck** (development): static analysis
- **shellcheck** (development): shell script linting
- **actionlint** (development): GitHub Actions workflow validation
