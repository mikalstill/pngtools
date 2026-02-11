#ifndef CHUNK_MEANINGS_H
#define CHUNK_MEANINGS_H

// PNG chunk name case-bit meanings.
// Each letter in a four-character chunk name encodes one property
// via its case (upper = first value, lower = second value).
char *chunk_meanings[4][2] = { { "Critical", "Ancillary" },
                               { "public", "private" },
                               { "PNG 1.2 compliant", "in reserved chunk space" },
                               { "unsafe to copy", "safe to copy" } };

#endif
