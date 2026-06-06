# Fuzzing pngtools

Coverage-guided fuzz harnesses for the parts of pngtools that touch
attacker-controlled bytes. Built with clang's libFuzzer + AddressSanitizer
+ UndefinedBehaviorSanitizer. Run nightly in CI; a 60-second smoke test
runs on every PR.

## Targets

- **fuzz_pngchunks** -- exercises `pngchunks_walk()`, the custom
  chunk-header parser in `pngchunks.c`. This is the most interesting
  target: it does pointer arithmetic over the attacker-controlled
  `chunk_len` field, which is the classic place to find integer
  overflows and missing bounds checks.

- **fuzz_pnginfo** -- exercises how `pnginfo.c` drives libpng (memory
  reader, `png_read_info`, IHDR / palette / text retrieval, full
  `png_read_image` allocation). libpng itself is fuzzed extensively
  by OSS-Fuzz, so this target is aimed at *our* usage of libpng,
  not libpng's internals.

## Building locally

You need `clang` (any recent version) and `libpng-dev`. The harnesses
do not need autotools.

```bash
cd fuzz
make            # builds fuzz_pngchunks and fuzz_pnginfo
make seed       # populates corpus/<target>/ from repo samples
```

The seed step copies the five top-level sample PNGs plus anything in
`testdata/`. Run `tests/.venv/bin/python tests/generate_test_images.py`
first if you want the generated images included.

## Running

libFuzzer's argv is `<corpus_dir> [seeds_dir...]`. The corpus dir
accumulates interesting inputs across runs; the seeds dirs are read
once at startup.

```bash
# 60 seconds of fuzzing on fuzz_pngchunks, growing corpus/fuzz_pngchunks
./fuzz_pngchunks \
    -max_total_time=60 \
    -dict=dictionary.txt \
    -max_len=1048576 \
    -rss_limit_mb=2048 \
    corpus/fuzz_pngchunks
```

A crash drops a `crash-<sha1>` file in the current directory. Reproduce
it by running the target with the crash file as a positional argument:

```bash
./fuzz_pngchunks crash-deadbeef...
```

`make clean` removes built binaries and crash artifacts but leaves
`corpus/` alone so you can keep mutating across rebuilds.

## CI

`.github/workflows/fuzz.yml` runs three modes:

- **pull_request**: 60s per target as a smoke test. Catches crashes
  reachable in seconds, and verifies that the harnesses still build.
- **schedule** (daily, 04:00 UTC): 30 minutes per target. This is where
  most real findings will come from.
- **workflow_dispatch**: configurable duration via the `duration` input.

Crashes and corpus snapshots are uploaded as workflow artifacts.

## Adding a new target

1. Write `fuzz_<thing>.c` exporting `LLVMFuzzerTestOneInput` and
   (optionally) `LLVMFuzzerInitialize`.
2. Add it to `TARGETS` in `Makefile`.
3. Add a `mkdir corpus/fuzz_<thing>` line to `seed-corpus.sh`.
4. Add it to the matrix in `.github/workflows/fuzz.yml`.
