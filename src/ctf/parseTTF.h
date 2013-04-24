/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_PARSETTF_H__
#define __LIBEOT_PARSETTF_H__

#include <stdint.h>

#include "SFNTContainer.h"

struct TTFheadData
{
  int16_t indexToLocFormat;
};

struct TTFmaxpData
{
  uint16_t numGlyphs;
  uint16_t maxPoints;
  uint16_t maxContours;
  uint16_t maxComponentPoints;
  uint16_t maxComponentContours;
  uint16_t maxZones;
  uint16_t maxTwilightPoints;
  uint16_t maxStorage;
  uint16_t maxFunctionDefs;
  uint16_t maxInstructionDefs;
  uint16_t maxStackElements;
  uint16_t maxSizeOfInstructions;
  uint16_t maxComponentElements;
  uint16_t maxComponentDepth;
};

enum EOTError TTFParseHead(struct SFNTTable *tbl, struct TTFheadData *out);

enum EOTError TTFParseMaxp(struct SFNTTable *tbl, struct TTFmaxpData *out);

#endif /* #define __LIBEOT_PARSETTF_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
