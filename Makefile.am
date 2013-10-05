bin_PROGRAMS = eot2ttf
lib_LTLIBRARIES = libeot.la
libeot_includedir = $(includedir)/libeot
libeot_include_HEADERS = inc/libeot.h


libeot_la_SOURCES = src/libeot.c inc/libeot.h src/EOT.c src/EOT.h src/EOTError.h src/writeFontFile.c src/flags.h src/triplet_encodings.c src/triplet_encodings.h src/writeFontFile.h src/ctf/parseCTF.c src/ctf/parseCTF.h src/ctf/parseTTF.c src/ctf/parseTTF.h src/ctf/SFNTContainer.c src/ctf/SFNTContainer.h src/util/logging.h src/util/max.h src/util/stream.h src/util/stream.c src/lzcomp/ahuff.c src/lzcomp/AHUFF.H src/lzcomp/bitio.c src/lzcomp/BITIO.H src/lzcomp/ERRCODES.H src/lzcomp/liblzcomp.c src/lzcomp/liblzcomp.h src/lzcomp/lzcomp.c src/lzcomp/LZCOMP.H src/lzcomp/mtxmem.c src/lzcomp/MTXMEM.H
eot2ttf_LDADD = libeot.la
eot2ttf_SOURCES = src/eot2ttf.c
common_flags = --std=c99 -DDECOMPRESS_ON
debug_flags = -Werror -Wall -g -O0 $(common_flags)
release_flags = -O2 $(common_flags)
if DEBUG
eot2ttf_CFLAGS = $(debug_flags)
libeot_la_CFLAGS = $(debug_flags)
else
eot2ttf_CFLAGS = $(release_flags)
libeot_la_CFLAGS = $(release_flags)
endif
