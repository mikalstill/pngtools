// libFuzzer harness for pngchunks_walk().
//
// This exercises the custom chunk-walking parser in pngchunks.c -- the
// only part of pngtools that parses PNG bytes without going through
// libpng. Bugs that matter here include pointer-arithmetic overflows
// on the attacker-controlled chunk length field, unaligned reads, and
// missing bounds checks on chunk payloads.

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "pngchunks.h"

int
LLVMFuzzerInitialize(int *argc, char ***argv)
{
  (void)argc;
  (void)argv;
  // The walker is called with verbose=0 so it shouldn't write
  // anything to stdout, but redirect anyway to defend against
  // future drift. Do not touch stderr: libFuzzer prints its
  // own status (and ASan/UBSan diagnostics) there.
  (void)freopen("/dev/null", "w", stdout);
  return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  pngchunks_walk(data, size, 0);
  return 0;
}
