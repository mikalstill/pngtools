# pngtools

A suite of command-line utilities for inspecting and manipulating PNG
image files, modelled on libtiff's tifftools. pngtools is packaged by
several Linux distributions.

## Tools

- **pnginfo**: Display detailed metadata about PNG files (dimensions,
  bit depth, colour type, interlacing, compression, text chunks, etc).
  Supports tiffinfo-compatible output labels (`-t`) and bitmap
  dumping (`-d`, `-D`).

- **pngchunks**: List the raw chunk structure of a PNG file by
  directly parsing chunk headers. Shows chunk type, length, CRC,
  IHDR fields, and the case-encoded properties of each chunk name.

- **pngchunkdesc**: Read PNG chunk names from stdin and decode the
  meaning of each letter's case (critical/ancillary, public/private,
  PNG 1.2 compliant, safe to copy).

- **pngcp**: Copy a PNG file while optionally changing the bit depth
  (`-d`) and number of samples per pixel (`-s`).

## Building

Requires libpng-dev and optionally docbook-utils (for man pages):

```bash
aclocal && autoconf && automake --add-missing && autoreconf
./configure
make
make install
```

## Usage Examples

```bash
# Show PNG metadata
pnginfo image.png

# Show metadata with tiffinfo-compatible labels
pnginfo -t image.png

# Dump image bitmap as hex
pnginfo -d image.png

# List raw chunk structure
pngchunks image.png

# Decode chunk name properties
echo "tEXt" | pngchunkdesc

# Copy with changed bit depth
pngcp -d 16 input.png output.png
```

## Testing

The test suite uses Python testtools + stestr:

```bash
# One-time setup
python3 -m venv tests/.venv
tests/.venv/bin/pip install -r test-requirements.txt

# Generate additional test images
tests/.venv/bin/python tests/generate_test_images.py

# Run all tests
tests/.venv/bin/stestr run

# Run a specific test module
tests/.venv/bin/stestr run test_pnginfo
```

Or use the all-in-one script that handles building and testing
(including first-time setup):

```bash
scripts/build-and-test.sh
```

### Pre-commit Hooks

Pre-commit hooks enforce formatting (clang-format), static
analysis (cppcheck), and run the full build-and-test cycle:

```bash
pip install pre-commit
pre-commit install
```

You can also run the format checker directly:

```bash
scripts/check-format.sh       # check only
scripts/check-format.sh fix   # auto-format in place
```

## Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) -- code structure, data flow,
  and known bugs
- [AGENTS.md](AGENTS.md) -- guidelines for AI agents working on
  this codebase
- Man pages are built from DocBook SGML sources in the `man/`
  directory

## License

GNU General Public License v2. See COPYING for details.

## Author

Michael Still (mikal@stillhq.com)
https://www.madebymikal.com/category/pngtools/
