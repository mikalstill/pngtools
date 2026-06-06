#ifndef PNGCHUNKS_H
#define PNGCHUNKS_H

#include <stddef.h>
#include <stdint.h>

extern const char pngchunks_magic[8];

// Walk the PNG chunk list in the buffer [data, data+size).
//
// Returns 0 if the buffer was walked successfully to an IEND chunk, or
// -1 if any structural problem was detected (not a PNG, truncated chunk
// header, chunk length running past the end of the buffer, etc).
//
// If verbose is non-zero, the walker prints chunk information to stdout
// and error diagnostics to stderr. If verbose is zero the walker is
// silent, which is what fuzz harnesses want.
int pngchunks_walk(const unsigned char *data, size_t size, int verbose);

#endif
