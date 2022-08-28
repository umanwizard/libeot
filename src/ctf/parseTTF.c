/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license,
 * version 2.0. For full details, see the file LICENSE
 */

#include "parseTTF.h"

#include <libeot/libeot.h>

#include "../util/stream.h"
#include "SFNTContainer.h"

enum EOTError TTFParseHead(struct SFNTTable *tbl, struct TTFheadData *out)
{
  if (tbl->bufSize < 52) {
    return EOT_CORRUPT_FILE;
  }
  *out = (struct TTFheadData){0};
  struct Stream s = constructStream(tbl->buf, tbl->bufSize);
  seekAbsolute(&s, 50);
  BEReadS16(&s, &out->indexToLocFormat);
  return EOT_SUCCESS;
}

enum EOTError TTFParseMaxp(struct SFNTTable *tbl, struct TTFmaxpData *out)
{
  struct Stream s = constructStream(tbl->buf, tbl->bufSize);
  enum StreamResult sResult;
  // sResult = seekRelative(&s, 4);
  memset(out, 0, sizeof(struct TTFmaxpData));
  uint32_t version;
  RD2(BEReadU32, &s, &version, sResult);
  RD2(BEReadU16, &s, &out->numGlyphs, sResult);
  if (version == 0x00010000) {
    RD2(BEReadU16, &s, &out->maxPoints, sResult);
    RD2(BEReadU16, &s, &out->maxContours, sResult);
    RD2(BEReadU16, &s, &out->maxComponentPoints, sResult);
    RD2(BEReadU16, &s, &out->maxComponentContours, sResult);
    RD2(BEReadU16, &s, &out->maxZones, sResult);
    RD2(BEReadU16, &s, &out->maxTwilightPoints, sResult);
    RD2(BEReadU16, &s, &out->maxStorage, sResult);
    RD2(BEReadU16, &s, &out->maxFunctionDefs, sResult);
    RD2(BEReadU16, &s, &out->maxInstructionDefs, sResult);
    RD2(BEReadU16, &s, &out->maxStackElements, sResult);
    RD2(BEReadU16, &s, &out->maxSizeOfInstructions, sResult);
    RD2(BEReadU16, &s, &out->maxComponentElements, sResult);
    RD2(BEReadU16, &s, &out->maxComponentDepth, sResult);
  }
  return EOT_SUCCESS;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
