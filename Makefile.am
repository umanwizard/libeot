bin_PROGRAMS = eot2ttf
eot2ttf_SOURCES = src/eot2ttf.c src/EOT.c src/EOT.h src/EOTError.h src/writeFontFile.c src/flags.h src/triplet_encodings.c src/triplet_encodings.h src/writeFontFile.h src/ctf/parseCTF.c src/ctf/parseCTF.h src/ctf/parseTTF.c src/ctf/parseTTF.h src/ctf/SFNTContainer.c src/ctf/SFNTContainer.h src/lzcomp/ahuff.c src/lzcomp/AHUFF.H src/lzcomp/bitio.c src/lzcomp/BITIO.H src/lzcomp/ERRCODES.H src/lzcomp/liblzcomp.c src/lzcomp/liblzcomp.h src/lzcomp/lzcomp.c src/lzcomp/LZCOMP.H src/lzcomp/mtxmem.c src/lzcomp/MTXMEM.H src/util/logging.h src/util/max.h src/util/stream.h src/util/stream.c
if DEBUG
AM_CFLAGS = -Werror -Wall -g -O0 --std=c99 -DDECOMPRESS_ON
else
AM_CFLAGS = -O2 --std=c99 -DDECOMPRESS_ON
endif
