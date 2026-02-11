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

48 automated tests using Python testtools + stestr:

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
  `pnginfo_displayfile`, `pnginfo_xmalloc`)
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
| configure.ac      | Autoconf configuration                       |
| Makefile.am       | Automake build rules                         |
| man/*.sgml        | DocBook SGML man page sources                |

## Priority Improvements

These are the most impactful bugs and improvements, roughly ordered
by severity. See ARCHITECTURE.md "Known Bugs and Issues" for full
details.

### Bugs to Fix First

1. **pngwrite.c:67 -- derive colour type from channels**: Replace the
   hardcoded `PNG_COLOR_TYPE_RGB` with logic that maps channel count
   to the correct PNG colour type (1=gray, 2=gray+alpha, 3=RGB,
   4=RGBA).

2. **pngcp.h type mismatch**: Align the width/height parameter types
   between the header (`unsigned long *`) and pngread.c's
   implementation (`png_uint_32 *`). Pick one and use it consistently.

3. **inflateraster.c:31 -- error return value**: Change
   `return (png_byte *) -1` to `return NULL` so the caller's NULL
   check in pngcp.c actually catches the error.

### Improvements Worth Making

4. **Deduplicate the meanings table**: The chunk name case-meanings
   array is defined identically in pngchunkdesc.c and pngchunks.c.
   Extract it to a shared header or source file.

5. **Check fread return values**: pnginfo.c:159 and pngread.c:28
   ignore the return value of `fread()` when reading the PNG
   signature.

6. **Fix resource leaks**: pnginfo.c's `pnginfo_error()` calls
   `exit(1)` without cleanup. Consider restructuring to close files
   and free libpng structures before exiting, or accept the leaks as
   intentional for a short-lived CLI tool and document that decision.

7. **Fix inflateraster limitations**: The two `todo_mikal` items --
   multi-byte sample support and combined bitdepth+channel changes --
   have been outstanding for ~20 years.

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
  builds the project and runs all 48 tests. PRs must pass CI.
