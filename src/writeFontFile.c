/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license,
 * version 2.0. For full details, see the file LICENSE
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libeot/libeot.h>

#include "ctf/parseCTF.h"
#include "lzcomp/liblzcomp.h"
#include "util/stream.h"
const uint8_t ENCRYPTION_KEY = 0x50;

enum EOTError writeFontBuffer(const uint8_t *font, unsigned fontSize,
                              bool compressed, bool encrypted,
                              uint8_t **finalOutBuffer,
                              unsigned *finalFontSize) {
  enum EOTError result;
  uint8_t *buf = (uint8_t *)malloc(fontSize);
  for (unsigned i = 0; i < fontSize; ++i) {
    if (encrypted) {
      buf[i] = font[i] ^ ENCRYPTION_KEY;
    } else {
      buf[i] = font[i];
    }
  }
  uint8_t *ctfs[3] = {NULL, NULL, NULL};
  struct SFNTContainer *ctr = NULL;
  if (compressed) {
#ifndef DONT_UNCOMPRESS
    unsigned sizes[3];
    struct Stream sBuf = constructStream(buf, fontSize);
    result = unpackMtx(&sBuf, fontSize, ctfs, sizes);
    if (result != EOT_SUCCESS) {
      goto CLEANUP;
    }
    struct Stream streams[3];
    for (unsigned i = 0; i < 3; ++i) {
      streams[i] = constructStream(ctfs[i], sizes[i]);
    }
    struct Stream *streamPtrs[3] = {streams, streams + 1,
                                    streams + 2}; /* ugh */
    result = parseCTF(streamPtrs, &ctr);
    if (result != EOT_SUCCESS) {
      goto CLEANUP;
    }
    result = dumpContainer(ctr, finalOutBuffer, finalFontSize);
    if (result != EOT_SUCCESS) {
      goto CLEANUP;
    }
#else
    *finalOutBuffer = buf;
    *finalFontSize = fontSize;
#endif
  } else {
    *finalOutBuffer = buf;
    *finalFontSize = fontSize;
  }
  result = EOT_SUCCESS;
CLEANUP:
  if (*finalOutBuffer != buf) {
    free(buf);
  }
  for (unsigned i = 0; i < 3; ++i) {
    free(ctfs[i]);
  }
  if (ctr) {
    freeContainer(ctr);
  }
  return result;
}

enum EOTError writeFontFile(const uint8_t *font, unsigned fontSize,
                            bool compressed, bool encrypted, FILE *outFile) {
  enum EOTError result;
  uint8_t *finalBuf = NULL;
  unsigned finalFontSize;
  result = writeFontBuffer(font, fontSize, compressed, encrypted, &finalBuf,
                           &finalFontSize);
  if (!result) {
    goto CLEANUP;
  }
  int itemsWritten = fwrite(finalBuf, 1, (long)finalFontSize, outFile);
  if (itemsWritten == finalFontSize) {
    result = EOT_SUCCESS;
  } else {
    result = EOT_FWRITE_ERROR;
  }
CLEANUP:
  if (finalBuf) {
    free(finalBuf);
  }
  return result;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
