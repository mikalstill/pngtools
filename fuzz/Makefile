# Build coverage-guided fuzz targets for pngtools.
#
# Targets are built with clang's libFuzzer, AddressSanitizer, and
# UndefinedBehaviorSanitizer. Run a target with:
#
#     ./fuzz_pngchunks -max_total_time=60 corpus/pngchunks seeds
#
# See README.md in this directory for the full workflow.

# clang is required: libFuzzer ships with clang's compiler-rt and is
# not available under gcc. Override with e.g. `make CC=clang-18` if
# you need a specific version.
CC = clang
SANITIZERS ?= address,undefined
FUZZ_CFLAGS = -O1 -g -fno-omit-frame-pointer \
              -fsanitize=fuzzer,$(SANITIZERS) \
              -I..

PNG_CFLAGS := $(shell pkg-config --cflags libpng 2>/dev/null)
PNG_LIBS := $(shell pkg-config --libs libpng 2>/dev/null)
ifeq ($(strip $(PNG_LIBS)),)
PNG_LIBS = -lpng
endif

TARGETS = fuzz_pngchunks fuzz_pnginfo

all: $(TARGETS)

fuzz_pngchunks: fuzz_pngchunks.c ../pngchunks.c ../pngchunks.h
	$(CC) $(FUZZ_CFLAGS) -DPNGCHUNKS_NO_MAIN \
	      -o $@ fuzz_pngchunks.c ../pngchunks.c -lm

fuzz_pnginfo: fuzz_pnginfo.c
	$(CC) $(FUZZ_CFLAGS) $(PNG_CFLAGS) \
	      -o $@ fuzz_pnginfo.c $(PNG_LIBS)

corpus:
	mkdir -p corpus/fuzz_pngchunks corpus/fuzz_pnginfo

seed: corpus
	./seed-corpus.sh

clean:
	rm -f $(TARGETS)
	rm -rf crash-* leak-* timeout-* oom-*

.PHONY: all corpus seed clean
