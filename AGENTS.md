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

There is no automated test suite. Five sample PNG files are included
in the repository root for manual testing:

```bash
./pnginfo sample.png
./pnginfo -t input.png
./pnginfo -d grayscale.png
./pngchunks sample.png
echo "IHDR" | ./pngchunkdesc
./pngcp input.png output.png
./pngcp -d 16 -s 3 grayscale.png output.png
```

When making changes, verify at minimum that all four tools build
cleanly and produce sensible output on the sample files.

## Code Style

- C with GNU Autotools conventions
- Indentation is a mix of tabs and spaces (historical; GNU style with
  tabs for primary indentation, two-space offsets for continuation)
- Functions use `snake_case` with tool-name prefixes (e.g.
  `pnginfo_displayfile`, `pnginfo_xmalloc`)
- Comments are C99 `//` style for inline, `/* */` for block headers
- DocBook documentation is embedded in source file headers as
  structured comments

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

1. **pnginfo.c:324 -- text[1] should be text[i]**: Wrong array index
   in the text chunk compression type display. One-character fix.

2. **pnginfo.c:205-219 -- uninitialized palette/transparency counts**:
   Need to call `png_get_PLTE()` and `png_get_tRNS()` to populate
   `num_palette` and `num_trans` before using them.

3. **pngwrite.c:67 -- derive colour type from channels**: Replace the
   hardcoded `PNG_COLOR_TYPE_RGB` with logic that maps channel count
   to the correct PNG colour type (1=gray, 2=gray+alpha, 3=RGB,
   4=RGBA).

4. **pngcp.h type mismatch**: Align the width/height parameter types
   between the header (`unsigned long *`) and pngread.c's
   implementation (`png_uint_32 *`). Pick one and use it consistently.

5. **inflateraster.c:31 -- error return value**: Change
   `return (char *) -1` to `return NULL` so the caller's NULL check
   in pngcp.c actually catches the error.

6. **pngchunks.c:68 -- mmap error check**: Compare against
   `MAP_FAILED` instead of `< 0`.

### Improvements Worth Making

7. **Add an automated test suite**: Even simple shell-script tests
   that run each tool on the sample files and compare output against
   golden files would catch regressions. The CI workflow currently
   only builds.

8. **Deduplicate the meanings table**: The chunk name case-meanings
   array is defined identically in pngchunkdesc.c and pngchunks.c.
   Extract it to a shared header or source file.

9. **Check fread return values**: pnginfo.c:159 and pngread.c:28
   ignore the return value of `fread()` when reading the PNG
   signature.

10. **Fix resource leaks**: pnginfo.c's `pnginfo_error()` calls
    `exit(1)` without cleanup. Consider restructuring to close files
    and free libpng structures before exiting, or accept the leaks as
    intentional for a short-lived CLI tool and document that decision.

11. **Fix inflateraster limitations**: The two `todo_mikal` items --
    multi-byte sample support and combined bitdepth+channel changes --
    have been outstanding for ~20 years.

## Things to Be Careful About

- **This is packaged by Linux distributions.** Changes to command-line
  interfaces, output formats, or exit codes may break downstream
  users and packaging scripts. Preserve existing behaviour unless
  intentionally fixing a bug.

- **The project has no tests.** Until a test suite exists, manually
  verify changes against all five sample PNG files and both
  pnginfo output modes (`-t` and default).

- **pngchunks does manual binary parsing.** It does not use libpng
  and instead memory-maps the file and walks chunk headers directly.
  Be especially careful with endianness (uses `ntohl`) and struct
  packing assumptions when modifying this code.

- **DocBook man pages live in man/*.sgml.** If you change a tool's
  command-line interface or behaviour, update the corresponding SGML
  man page source as well.

- **The CI workflow uses actions/checkout@v2.** This is outdated and
  may eventually stop working. Consider updating to v4 when touching
  CI.
