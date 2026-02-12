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
