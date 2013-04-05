/* Copyright (c) 2012 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is license under the MIT license.
 * For full details, see the file LICENSE
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "EOTError.h"
#include "util/stream.h"
#include "lzcomp/liblzcomp.h"
#include "ctf/parseCTF.h"
const uint8_t ENCRYPTION_KEY = 0x50;

enum EOTError writeFontFile(uint8_t *font, unsigned fontSize, bool compressed, bool encrypted, FILE *outFile)
{
  uint8_t *buf = (uint8_t *)malloc(fontSize);
  for (unsigned i = 0; i < fontSize; ++i)
  {
    if (encrypted)
    {
      buf[i] = font[i] ^ ENCRYPTION_KEY;
    }
    else
    {
      buf[i] = font[i];
    }
  }
  uint8_t *finalBuf;
  long finalFontSize;
  if (compressed)
  {
    uint8_t *ctfs[3];
    unsigned sizes[3];
    struct Stream sBuf = constructStream(buf, fontSize);
    enum EOTError result = unpackMtx(&sBuf, fontSize, ctfs, sizes);
    struct Stream streams[3];
    for (unsigned i = 0; i < 3; ++i)
    {
      streams[i] = constructStream(ctfs[i], sizes[i]);
    }
    struct Stream *streamPtrs[3] = {streams, streams + 1, streams + 2}; /* ugh */
    if (result != EOT_SUCCESS)
    {
      return result;
    }
    struct SFNTContainer *ctr;
    result = parseCTF(streamPtrs, &ctr);
    if (result != EOT_SUCCESS)
    {
      return result;
    }
    result = dumpContainer(ctr, &finalBuf);
    if (result != EOT_SUCCESS)
    {
      return result;
    }
    /* FIXME MEMORY */
  }
  else
  {
    finalBuf = buf;
    finalFontSize = fontSize;
  }
  fwrite(finalBuf, 1, finalFontSize, outFile);
  if (finalBuf != buf)
  {
    free(finalBuf);
  }
  free(buf);
  return EOT_SUCCESS;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
