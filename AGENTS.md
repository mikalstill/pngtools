# AGENTS.md -- Guidelines for AI Agents Working on pngtools

## Project Summary

pngtools is a small C project (~1,100 lines across 7 source files)
providing four command-line PNG utilities: pnginfo, pngchunks,
pngchunkdesc, and pngcp. It uses libpng for PNG I/O and GNU Autotools
for building. See ARCHITECTURE.md for detailed structure and known
bugs.

## Build Instructions

```bash
aclocal && autoconf && automake --add-missing && autoreconf
./configure
make
```

Dependencies: libpng-dev, libm, docbook-utils (for man pages).

## Testing

53 automated tests using Python testtools + stestr:

```bash
# One-time setup
python3 -m venv tests/.venv
tests/.venv/bin/pip install -r test-requirements.txt
tests/.venv/bin/python tests/generate_test_images.py

# Run all tests (requires binaries to be built first)
tests/.venv/bin/stestr run

# Run a specific test module
tests/.venv/bin/stestr run test_pnginfo
```

Tests run the compiled binaries via subprocess and check exit
codes and stdout/stderr content. They use sample PNGs from the
repo root plus generated images in `testdata/`.

When making changes, run the full test suite. If adding new
features or fixing bugs, add corresponding tests.

A pre-commit hook runs the full build-and-test cycle
automatically. Set it up with:

```bash
pip install pre-commit
pre-commit install
```

Or run the build and tests manually via
`scripts/build-and-test.sh`.

## Code Style

- C with GNU Autotools conventions
- Indentation is a mix of tabs and spaces (historical; GNU style with
  tabs for primary indentation, two-space offsets for continuation)
- Functions use `snake_case` with tool-name prefixes (e.g.
  `pnginfo_displayfile`)
- Comments are C99 `//` style for inline, `/* */` for block headers
- DocBook documentation is embedded in source file headers as
  structured comments
- The build uses `-Wall -Wextra -Werror`: all warnings are errors.
  New code must compile warning-free.

## Key Files

| File              | Purpose                                      |
|-------------------|----------------------------------------------|
| pnginfo.c         | PNG metadata display tool (main tool)        |
| pngchunks.c       | Raw PNG chunk structure lister               |
| pngchunkdesc.c    | Chunk name case-bit decoder                  |
| pngcp.c           | PNG copy tool entry point                    |
| pngread.c         | PNG reading helper (readimage)               |
| pngwrite.c        | PNG writing helper (writeimage)              |
| inflateraster.c   | Raster bitdepth/channel transformation       |
| pngcp.h           | Header for pngcp helper functions            |
| chunk_meanings.h  | Shared chunk name case-bit meanings table     |
| configure.ac      | Autoconf configuration                       |
| Makefile.am       | Automake build rules                         |
| man/*.sgml        | DocBook SGML man page sources                |

## Things to Be Careful About

- **This is packaged by Linux distributions.** Changes to command-line
  interfaces, output formats, or exit codes may break downstream
  users and packaging scripts. Preserve existing behaviour unless
  intentionally fixing a bug.

- **Run the test suite before committing.** The pre-commit hook
  does this automatically. If tests fail, fix them before pushing.

- **pngchunks does manual binary parsing.** It does not use libpng
  and instead memory-maps the file and walks chunk headers directly.
  Be especially careful with endianness (uses `ntohl`) and struct
  packing assumptions when modifying this code.

- **DocBook man pages live in man/*.sgml.** If you change a tool's
  command-line interface or behaviour, update the corresponding SGML
  man page source as well.

- **CI runs the full test suite.** The GitHub Actions workflow
  builds the project and runs all 53 tests. PRs must pass CI.
