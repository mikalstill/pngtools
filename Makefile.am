SUBDIRS = man

AM_CFLAGS = -Wall -Wextra -Werror

bin_PROGRAMS = pnginfo pngcp pngchunkdesc pngchunks

pnginfo_SOURCES = pnginfo.c
pngcp_SOURCES = pngcp.c pngread.c pngwrite.c inflateraster.c
pngchunkdesc_SOURCES = pngchunkdesc.c
pngchunks_SOURCES = pngchunks.c

noinst_HEADERS = pngcp.h chunk_meanings.h

pnginfo_LDADD = -lpng
pngcp_LDADD = -lpng -lm

EXTRA_DIST = \
	.stestr.conf \
	test-requirements.txt \
	sample.png \
	input.png \
	foursamplesperpixel.png \
	grayscale.png \
	multibytesample.png \
	tests/__init__.py \
	tests/base.py \
	tests/generate_test_images.py \
	tests/test_pngchunkdesc.py \
	tests/test_pngchunks.py \
	tests/test_pngcp.py \
	tests/test_pnginfo.py \
	configure \
	config.h.in
