/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license,
 * version 2.0. For full details, see the file LICENSE
 */

#include <libeot/libeot.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../triplet_encodings.h"
#include "../util/logging.h"
#include "../util/max.h"
#include "../util/stream.h"
#include "SFNTContainer.h"
#include "parseTTF.h"

struct SFNTOffsetTable {
  uint32_t scalarType;
  uint16_t numTables;
  uint16_t searchRange;
  uint16_t entrySelector;
  uint16_t rangeShift;
};

enum StreamResult parseOffsetTable(struct Stream *s,
                                   struct SFNTOffsetTable *tbl)
{
  enum StreamResult res;
  RD(BEReadU32, s, &tbl->scalarType, res);
  RD(BEReadU16, s, &tbl->numTables, res);
  RD(BEReadU16, s, &tbl->searchRange, res);
  RD(BEReadU16, s, &tbl->entrySelector, res);
  RD(BEReadU16, s, &tbl->rangeShift, res);
  return EOT_STREAM_OK;
}
/*
enum EOTError populateHdmx(struct SFNTTable *hdmx, struct TTFmaxpData
*maxpData, struct TTFheadData *headData, struct TTFhmtxData *hmtxData, struct
Stream *s)
{
}*/

enum StreamResult _ucvt_rdVal(struct Stream *sIn, int16_t *lastValue)
{
  uint8_t code, b2;
  enum StreamResult sResult = BEReadU8(sIn, &code);
  int16_t val;
  CHK_RD(sResult);
  if (code >= 248) {
    sResult = BEReadU8(sIn, &b2);
    CHK_RD(sResult);
    val = 238 * (code - 247) + b2;
  } else if (code >= 239) {
    sResult = BEReadU8(sIn, &b2);
    CHK_RD(sResult);
    val = -1 * (238 * (code - 239) + b2);
  } else if (code == 238) {
    sResult = BEReadS16(sIn, &val);
    CHK_RD(sResult);
  } else {
    val = code;
  }
  *lastValue += val; /* The CVT table in CTF format is set up so that this does
                        the right thing even if it overflows. */
  /* Unless someone tries to run this code on some horrible system that doesn't
   * use twos complement... */
  return EOT_STREAM_OK;
}

enum EOTError unpackCVT(struct SFNTTable *out, struct Stream *sIn)
{
  enum StreamResult sResult = seekAbsolute(sIn, out->offset);
  if (sResult != EOT_STREAM_OK) {
    return EOT_CORRUPT_FILE;
  }
  uint16_t tableLength;
  RD2(BEReadU16, sIn, &tableLength, sResult);
  struct Stream sOut = constructStream(NULL, 0);
  sResult = reserve(&sOut, tableLength * sizeof(int16_t));
  CHK_RD2(sResult);
  int16_t lastValue = 0;
  for (unsigned i = 0; i < tableLength; ++i) {
    sResult = _ucvt_rdVal(sIn, &lastValue);
    CHK_RD2(sResult);
    sResult = BEWriteS16(&sOut, lastValue);
    if (sResult != EOT_STREAM_OK) {
      return EOT_LOGIC_ERROR;
    }
  }
  out->buf = sOut.buf;
  out->bufSize = sOut.size;
  return EOT_SUCCESS;
}

/* http://www.w3.org/Submission/MTX/#id_255USHORT */
enum StreamResult read255UShort(struct Stream *sIn, uint16_t *out)
{
  uint8_t code, val1;
  enum StreamResult sResult;
  RD(BEReadU8, sIn, &code, sResult);
  switch (code) {
  case 253:
    sResult = BEReadU16(sIn, out);
    return sResult;
  case 255:
    RD(BEReadU8, sIn, &val1, sResult);
    *out = 253 + val1;
    return EOT_STREAM_OK;
  case 254:
    RD(BEReadU8, sIn, &val1, sResult);
    *out = 506 + val1;
    return EOT_STREAM_OK;
  default:
    *out = code;
    return EOT_STREAM_OK;
  }
}

/* http://www.w3.org/Submission/MTX/#id_255SHORT */
enum StreamResult read255Short(struct Stream *sIn, int16_t *out)
{
  uint8_t code;
  enum StreamResult sResult = BEReadU8(sIn, &code);
  CHK_RD(sResult);
  if (code == 253) {
    sResult = BEReadS16(sIn, out);
    return sResult;
  }
  int sign = 1;
  if (code == 250) {
    sign = -1;
    sResult = BEReadU8(sIn, &code);
    CHK_RD(sResult);
  }
  uint8_t out8;
  switch (code) {
  case 255:
    sResult = BEReadU8(sIn, &out8);
    CHK_RD(sResult);
    *out = 250 + out8;
    break;
  case 254:
    sResult = BEReadU8(sIn, &out8);
    CHK_RD(sResult);
    *out = 250 * 2 + out8;
    break;
  default:
    *out = code;
  }
  *out *= sign;
  return EOT_STREAM_OK;
}

/* This enum and the following functions prefaced by _dpi_ should only be used
 * by the decodePushInstructions function */
enum _dpi_TypeRead { BYTE, SHORT };

enum StreamResult _dpi_dump(struct Stream *out, enum _dpi_TypeRead *lastRead,
                            unsigned *typeLastReadCount, int16_t *data,
                            unsigned *dataIndex)
{
  enum StreamResult sResult;
#define NPUSHB 0x40
#define NPUSHW 0x41
#define PUSHB 0xB0
#define PUSHW 0xB8
  if (*typeLastReadCount > 0) {
    /* write stuff out */
    if (*typeLastReadCount < 8) {
      uint8_t op = ((*lastRead == BYTE) ? PUSHB : PUSHW) |
                   (uint8_t)(*typeLastReadCount - 1);
      sResult = BEWriteU8(out, op);
      CHK_RD(sResult);
    } else {
      uint8_t op = (*lastRead == BYTE) ? NPUSHB : NPUSHW;
      sResult = BEWriteU8(out, op);
      CHK_RD(sResult);
      sResult = BEWriteU8(out, (uint8_t)*typeLastReadCount);
      CHK_RD(sResult);
    }
    for (unsigned i = 0; i < *typeLastReadCount; ++i) {
      if (*lastRead == BYTE) {
        sResult = BEWriteU8(
            out, (uint8_t)(data[*dataIndex - *typeLastReadCount + i]));
        CHK_RD(sResult);
      } else {
        sResult = BEWriteS16(out, data[*dataIndex - *typeLastReadCount + i]);
        CHK_RD(sResult);
      }
    }
  }
  return EOT_STREAM_OK;
}

enum StreamResult _dpi_put(int16_t value, struct Stream *out,
                           enum _dpi_TypeRead *lastRead,
                           unsigned *typeLastReadCount, int16_t *data,
                           unsigned *dataIndex)
{
  enum StreamResult sResult;
  enum _dpi_TypeRead newType = (value >= 0 && value < 256) ? BYTE : SHORT;
  if (newType != *lastRead || *typeLastReadCount == 255) {
    sResult = _dpi_dump(out, lastRead, typeLastReadCount, data, dataIndex);
    if (sResult != EOT_STREAM_OK) {
      return sResult;
    }
    *lastRead = newType;
    *typeLastReadCount = 0;
  }
  data[(*dataIndex)++] = value;
  ++*typeLastReadCount;
  return EOT_STREAM_OK;
}

/* http://www.w3.org/Submission/MTX/#HopCodes */
enum EOTError decodePushInstructions(struct Stream *sIn, struct Stream *sOut,
                                     unsigned pushCount)
{
  enum StreamResult sResult;
  unsigned remaining = pushCount;
  enum _dpi_TypeRead typeLastRead =
      BYTE; /* doesn't actually matter what this is initialized to since it
               will just get reset with no effect if the first thing is a
               short */
  unsigned typeLastReadCount = 0;
  unsigned dataIndex = 0;
  int16_t *data = (int16_t *)malloc(sizeof(int16_t) * pushCount);
  enum EOTError returnedStatus;
  if (!data) {
    return EOT_CANT_ALLOCATE_MEMORY;
  }
  while (remaining) {
    uint8_t code;
    sResult = BEPeekU8(sIn, &code);
    CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
    int16_t val, prev;
    switch (code) {
    case 0xFB:
      /* A B 0xFB C -> A B A C A */
      if (remaining < 3) {
        returnedStatus = EOT_CORRUPT_HOPCODE_DATA;
        goto CLEANUP;
      }
      remaining -= 3;
      if (dataIndex < 2) {
        returnedStatus = EOT_CORRUPT_HOPCODE_DATA;
        goto CLEANUP;
      }
      prev = data[dataIndex - 2];
      BEReadU8(sIn, &code);
      sResult = _dpi_put(prev, sOut, &typeLastRead, &typeLastReadCount, data,
                         &dataIndex);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      sResult = read255Short(sIn, &val);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      sResult = _dpi_put(val, sOut, &typeLastRead, &typeLastReadCount, data,
                         &dataIndex);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      sResult = _dpi_put(prev, sOut, &typeLastRead, &typeLastReadCount, data,
                         &dataIndex);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      break;
    case 0xFC:
      /* A B 0xFB C D -> A B A C A D A */
      if (remaining < 5) {
        returnedStatus = EOT_CORRUPT_HOPCODE_DATA;
        goto CLEANUP;
      }
      remaining -= 5;
      if (dataIndex < 2) {
        returnedStatus = EOT_CORRUPT_HOPCODE_DATA;
        goto CLEANUP;
      }
      prev = data[dataIndex - 2];
      BEReadU8(sIn, &code);
      sResult = _dpi_put(prev, sOut, &typeLastRead, &typeLastReadCount, data,
                         &dataIndex);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      sResult = read255Short(sIn, &val);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      sResult = _dpi_put(val, sOut, &typeLastRead, &typeLastReadCount, data,
                         &dataIndex);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      sResult = _dpi_put(prev, sOut, &typeLastRead, &typeLastReadCount, data,
                         &dataIndex);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      sResult = read255Short(sIn, &val);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      sResult = _dpi_put(val, sOut, &typeLastRead, &typeLastReadCount, data,
                         &dataIndex);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      sResult = _dpi_put(prev, sOut, &typeLastRead, &typeLastReadCount, data,
                         &dataIndex);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      break;
    default:
      sResult = read255Short(sIn, &val);
      CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
      sResult = _dpi_put(val, sOut, &typeLastRead, &typeLastReadCount, data,
                         &dataIndex);
      --remaining;
      break;
    }
  }
  sResult =
      _dpi_dump(sOut, &typeLastRead, &typeLastReadCount, data, &dataIndex);
  CHK_CN(sResult, EOT_SECOND_STREAM_INCOMPLETE);
  returnedStatus = EOT_SUCCESS;
CLEANUP:
  free(data);
  return returnedStatus;
}

uint8_t _dsg_makeFlags(int16_t x, int16_t y, bool onCurve, bool firstTime)
{
  const uint8_t FLG_ON_CURVE = 0x01;
  const uint8_t FLG_X_SHORT = 0x02;
  const uint8_t FLG_Y_SHORT = 0x04;
  const uint8_t FLG_X_SAME = 0x10;
  const uint8_t FLG_Y_SAME = 0x20;
  uint8_t ret = 0;
  if (onCurve) {
    ret |= FLG_ON_CURVE;
  }
  if (!firstTime && (x == 0)) {
    ret |= FLG_X_SAME;
  } else {
    if (-256 < x && x < 0) {
      ret |= FLG_X_SHORT;
    } else if (0 <= x && x < 256) {
      ret |= FLG_X_SHORT;
      ret |= FLG_X_SAME; /* here means that X is positive. */
    }
  }
  if (!firstTime && (y == 0)) {
    ret |= FLG_Y_SAME;
  } else {
    if (-256 < y && y < 0) {
      ret |= FLG_Y_SHORT;
    } else if (0 <= y && y < 256) {
      ret |= FLG_Y_SHORT;
      ret |= FLG_Y_SAME; /* here means that Y is positive. */
    }
  }
  return ret;
}
enum EOTError decodeSimpleGlyph(int16_t numContours, struct Stream **streams,
                                struct Stream *out, bool calculateBoundingBox,
                                int16_t minX, int16_t minY, int16_t maxX,
                                int16_t maxY)
{
  if (numContours == 0) {
    return EOT_SUCCESS;
  }
  struct Stream *in = streams[0];
  enum StreamResult sResult;
  enum EOTError returnedStatus = EOT_SUCCESS;
  unsigned boundingBoxLocation;
  RD2(BEWriteS16, out, numContours, sResult);
  if (calculateBoundingBox) {
    boundingBoxLocation = out->pos;
    sResult = seekRelativeThroughReserve(out, 4 * sizeof(int16_t));
    CHK_RD2(sResult);
    minX = INT16_MAX, minY = INT16_MAX, maxX = INT16_MIN, maxY = INT16_MIN;
  } else {
    RD2(BEWriteS16, out, minX, sResult);
    RD2(BEWriteS16, out, minY, sResult);
    RD2(BEWriteS16, out, maxX, sResult);
    RD2(BEWriteS16, out, maxY, sResult);
  }
  unsigned totalPoints = 0;
  for (unsigned i = 0; i < (unsigned)numContours; ++i) {
    if (i == 0) {
      totalPoints = 1;
    }
    uint16_t pointsInContour;
    RD2(read255UShort, in, &pointsInContour, sResult);
    totalPoints += pointsInContour;
    RD2(BEWriteS16, out, totalPoints - 1, sResult);
  }
  uint8_t *flags = NULL;
  int16_t *xCoords = NULL, *yCoords = NULL;
  flags = (uint8_t *)malloc(totalPoints * sizeof(uint8_t));
  xCoords = (int16_t *)malloc(totalPoints * sizeof(int16_t));
  yCoords = (int16_t *)malloc(totalPoints * sizeof(int16_t));
  if (!flags || !xCoords || !yCoords) {
    returnedStatus = EOT_CANT_ALLOCATE_MEMORY;
    goto CLEANUP;
  }
  /* Read X-Y coordinates in shitty format described here:
   * http://www.w3.org/Submission/MTX/#TripletEncoding First flags and then
   * actual coordinates. */
  for (unsigned i = 0; i < totalPoints; ++i) {
    sResult = BEReadU8(in, &flags[i]);
    CHK_CN(sResult, EOT_CANT_ALLOCATE_MEMORY);
  }
  unsigned currX = 0, currY = 0;
  for (unsigned i = 0; i < totalPoints; ++i) {
    struct TripletEncoding enc = tripletEncodings[flags[i] & 0x7F];
    unsigned moreBytes = enc.byteCount - 1;
    if (in->pos + moreBytes > in->size) {
      returnedStatus = EOT_CORRUPT_FILE;
      goto CLEANUP;
    }
    struct Stream coords = constructStream(in->buf + in->pos, moreBytes);
    uint32_t dx, dy;
    sResult = readNBits(&coords, &dx, enc.xBits);
    CHK_CN(sResult, EOT_LOGIC_ERROR);
    sResult = readNBits(&coords, &dy, enc.yBits);
    CHK_CN(sResult, EOT_LOGIC_ERROR);
    if (coords.pos != coords.size || coords.bitPos != 0) {
      returnedStatus = EOT_LOGIC_ERROR;
      goto CLEANUP;
    }
    sResult = seekRelative(in, coords.size);
    CHK_CN(sResult, EOT_LOGIC_ERROR);
    xCoords[i] = enc.xSign * (dx + enc.deltaX);
    currX += xCoords[i];
    yCoords[i] = enc.ySign * (dy + enc.deltaY);
    currY += yCoords[i];
    minX = i16min(minX, currX);
    maxX = i16max(maxX, currX);
    minY = i16min(minY, currY);
    maxY = i16max(maxY, currY);
  }
  /* Coordinates are known now, but we need to handle instructions before they
   * can be output. */
  /* advance past the code size output */
  unsigned codeSizeLocation = out->pos;
  sResult = seekRelativeThroughReserve(out, sizeof(uint16_t));
  CHK_CN(sResult, EOT_CORRUPT_FILE);
  /* decode the push instructions for the glyph */
  uint16_t pushCount;
  sResult = read255UShort(in, &pushCount);
  CHK_CN(sResult, EOT_CORRUPT_FILE);
  enum EOTError result = decodePushInstructions(streams[1], out, pushCount);
  if (result != EOT_SUCCESS && result < EOT_WARN) {
    returnedStatus = result;
    goto CLEANUP;
  }
  /* copy over the rest of the instructions for the glyph */
  uint16_t codeSize;
  sResult = read255UShort(in, &codeSize);
  CHK_CN(sResult, EOT_CORRUPT_FILE);
  sResult = streamCopy(streams[2], out, codeSize);
  CHK_CN(sResult, EOT_CORRUPT_FILE);
  /* the below will be zero if we didn't go through the if (numContours > 0)
   * block. */
  unsigned unpackedCodeSize = out->pos - (codeSizeLocation + sizeof(uint16_t));
  /* FIXME: Figure out if there is a huge savings from using the 'repeat' flag
   * and if so, use it. (but I kinda doubt there is.) */
  for (unsigned i = 0; i < totalPoints; ++i) {
    uint8_t outFlags =
        _dsg_makeFlags(xCoords[i], yCoords[i], !(flags[i] & 0x80), i == 0);
    sResult = BEWriteU8(out, outFlags);
    CHK_CN(sResult, EOT_UNKNOWN_BUFFER_WRITE_ERROR);
  }
  for (unsigned i = 0; i < totalPoints; ++i) {
    int16_t x = xCoords[i];
    if (i == 0 || x != 0) {
      if (-256 < x && x < 0) {
        x *= -1;
      }
      if (0 <= x && x < 256) {
        sResult = BEWriteU8(out, (uint8_t)x);
      } else {
        sResult = BEWriteS16(out, x);
      }
      CHK_CN(sResult, EOT_UNKNOWN_BUFFER_WRITE_ERROR);
    }
  }
  for (unsigned i = 0; i < totalPoints; ++i) {
    int16_t y = yCoords[i];
    if (i == 0 || y != 0) {
      if (-256 < y && y < 0) {
        y *= -1;
      }
      if (0 <= y && y < 256) {
        sResult = BEWriteU8(out, (uint8_t)y);
      } else {
        sResult = BEWriteS16(out, y);
      }
      CHK_CN(sResult, EOT_UNKNOWN_BUFFER_WRITE_ERROR);
    }
  }
  unsigned currPos = out->pos;
  sResult = seekAbsoluteThroughReserve(out, codeSizeLocation);
  CHK_RD2(sResult);
  RD2(BEWriteU16, out, (uint16_t)unpackedCodeSize, sResult);
  CHK_RD2(sResult);
  sResult = seekAbsoluteThroughReserve(out, currPos);
  CHK_RD2(sResult);
  if (calculateBoundingBox) {
    unsigned endPos = out->pos;
    sResult = seekAbsoluteThroughReserve(out, boundingBoxLocation);
    CHK_RD2(sResult);
    RD2(BEWriteS16, out, minX, sResult);
    RD2(BEWriteS16, out, minY, sResult);
    RD2(BEWriteS16, out, maxX, sResult);
    RD2(BEWriteS16, out, maxY, sResult);
    sResult = seekAbsoluteThroughReserve(out, endPos);
  }
  returnedStatus = EOT_SUCCESS;
CLEANUP:
  free(flags);
  free(xCoords);
  free(yCoords);
  return returnedStatus;
}

enum EOTError decodeCompositeGlyph(struct Stream **streams, struct Stream *out)
{
  const uint16_t FLG_ARGS_WORDS = 0x1, FLG_HAVE_SCALE = 0x8,
                 FLG_MORE_COMPONENTS = 0x20, FLG_HAVE_XY_SCALE = 0x40,
                 FLG_HAVE_2_BY_2 = 0x80, FLG_HAVE_INSTR = 0x100;
  /* we don't need to interpret very much here, just the flags to know how much
   * to pass along into the output. */
  struct Stream *in = streams[0];
  int16_t minX, minY, maxX, maxY;
  enum StreamResult sResult;
  RD2(BEWriteS16, out, -1, sResult);

  RD2(BEReadS16, in, &minX, sResult);
  RD2(BEReadS16, in, &minY, sResult);
  RD2(BEReadS16, in, &maxX, sResult);
  RD2(BEReadS16, in, &maxY, sResult);
  RD2(BEWriteS16, out, minX, sResult);
  RD2(BEWriteS16, out, minY, sResult);
  RD2(BEWriteS16, out, maxX, sResult);
  RD2(BEWriteS16, out, maxY, sResult);
  uint16_t flags;
  do {
    RD2(BEReadU16, in, &flags, sResult);
    RD2(BEWriteU16, out, flags, sResult);
    /* for glyph index */
    sResult = streamCopy(in, out, 2);
    CHK_RD2(sResult);
    unsigned argsLength = (flags & FLG_ARGS_WORDS) ? 4 : 2;
    sResult = streamCopy(in, out, argsLength);
    CHK_RD2(sResult);
    unsigned transformBytes = 0;
    if (flags & FLG_HAVE_2_BY_2) {
      transformBytes = 8;
    } else if (flags & FLG_HAVE_XY_SCALE) {
      transformBytes = 4;
    } else if (flags & FLG_HAVE_SCALE) {
      transformBytes = 2;
    }
    sResult = streamCopy(in, out, transformBytes);
    CHK_RD2(sResult);
  } while (flags & FLG_MORE_COMPONENTS);
  if (flags & FLG_HAVE_INSTR) {
    /*
    https://learn.microsoft.com/en-us/typography/opentype/spec/glyf 
    uint16 numInstr
    */
    uint16_t numInstr = 0;
    unsigned numInstrLocation = out->pos;
    sResult = seekRelativeThroughReserve(out, sizeof(uint16_t));
    CHK_RD2(sResult);
    
    /* decode the push instructions for the glyph */
    uint16_t pushCount;
    sResult = read255UShort(in, &pushCount);
    CHK_RD2(sResult);
    enum EOTError result = decodePushInstructions(streams[1], out, pushCount);
    if (result != EOT_SUCCESS && result < EOT_WARN) {
      return result;
    }
    /* copy over the rest of the instructions for the glyph */
    uint16_t codeSize;
    sResult = read255UShort(in, &codeSize);
    CHK_RD2(sResult);
    sResult = streamCopy(streams[2], out, codeSize);
    CHK_RD2(sResult);

    numInstr = out->pos - (numInstrLocation + sizeof(uint16_t));
    if (numInstr > 0){
      unsigned currPos = out->pos;
      sResult = seekAbsoluteThroughReserve(out, numInstrLocation);
      CHK_RD2(sResult);
      RD2(BEWriteU16, out, (uint16_t)numInstr, sResult);
      CHK_RD2(sResult);
      sResult = seekAbsoluteThroughReserve(out, currPos);
      CHK_RD2(sResult);
    }
  }
  return EOT_SUCCESS;
}

enum EOTError decodeGlyph(struct Stream **streams, struct Stream *out)
{
  struct Stream *in = streams[0];
  int16_t numContours, xMin = 0, yMin = 0, xMax = 0, yMax = 0;
  bool calculateBoundingBox = false;
  enum StreamResult sResult;
  RD2(BEReadS16, in, &numContours, sResult);
  if (numContours < 0) {
    enum EOTError result = decodeCompositeGlyph(streams, out);
    if (result != EOT_SUCCESS && result < EOT_WARN) {
      return result;
    }
  } else {
    if (numContours == 0x7FFF) {
      /* Read real numContours and bounding box info. */
      RD2(BEReadS16, in, &numContours, sResult);
      RD2(BEReadS16, in, &xMin, sResult);
      RD2(BEReadS16, in, &yMin, sResult);
      RD2(BEReadS16, in, &xMax, sResult);
      RD2(BEReadS16, in, &yMax, sResult);
    } else {
      /* otherwise, calculate bounding box info ourselves. */
      calculateBoundingBox = true;
    }
    enum EOTError result =
        decodeSimpleGlyph(numContours, streams, out, calculateBoundingBox, xMin,
                          yMin, xMax, yMax);
    if (result != EOT_SUCCESS && result < EOT_WARN) {
      return result;
    }
  }
  return EOT_SUCCESS;
}

/* https://developer.apple.com/fonts/TTRefMan/RM06/Chap6glyf.html
 * http://www.w3.org/Submission/MTX/#CTFGlyph */
enum EOTError populateGlyfAndLoca(struct SFNTTable *glyf,
                                  struct SFNTTable *loca,
                                  struct TTFheadData *headData,
                                  struct TTFmaxpData *maxpData,
                                  struct Stream **streams)
{
  struct Stream *sCTF = streams[0];
  enum StreamResult sResult = seekAbsolute(sCTF, glyf->offset);
  if (sResult != EOT_STREAM_OK) {
    return EOT_CORRUPT_FILE;
  }
  bool overranAllocatedSpace = false;
  bool notEnoughGlyphs = false;
  seekAbsolute(streams[1], 0);
  seekAbsolute(streams[2], 0);
  unsigned maxSimpleGlyphSize = 10 + 2 * maxpData->maxContours + 2 +
                                maxpData->maxSizeOfInstructions +
                                maxpData->maxPoints * 5;
  unsigned maxCompoundGlyphSize = 26 + maxpData->maxSizeOfInstructions;
  unsigned maxGlyphSize = umax(maxSimpleGlyphSize, maxCompoundGlyphSize);
  unsigned maxTableSize = maxpData->numGlyphs * maxGlyphSize;
  struct Stream sOut = constructStream(NULL, 0);
  reserve(&sOut, maxTableSize);
  struct Stream sLocaOut = constructStream(NULL, 0);
  bool shortLoca = !(headData->indexToLocFormat);
  if (shortLoca) {
    reserve(&sLocaOut, 2 * (maxpData->numGlyphs + 1));
    BEWriteU16(&sLocaOut, 0);
  } else {
    reserve(&sLocaOut, 4 * (maxpData->numGlyphs + 1));
    BEWriteU32(&sLocaOut, 0);
  }
  for (unsigned i = 0; i < maxpData->numGlyphs; ++i) {
    // decode a glyph outline
    enum EOTError result = decodeGlyph(streams, &sOut);
    if (result != EOT_SUCCESS) {
      return result;
    }
    /* do padding */
    if (sOut.pos % 2) {
      BEWriteU8(&sOut, 0);
    }
    /* add an entry to the location table */
    if (shortLoca) {
      BEWriteU16(&sLocaOut, (uint16_t)(sOut.pos / 2));
    } else {
      BEWriteU32(&sLocaOut, sOut.pos);
    }
  }
  glyf->buf = sOut.buf;
  glyf->bufSize = sOut.size;
  loca->buf = sLocaOut.buf;
  loca->bufSize = sLocaOut.size;
  if (notEnoughGlyphs) {
    return EOT_WARN_NOT_ENOUGH_GLYPHS;
  }
  if (overranAllocatedSpace) {
    return EOT_WARN_NOT_ENOUGH_SPACE_RESERVED;
  }
  return EOT_SUCCESS;
}

enum EOTError parseCTF(struct Stream **streams, struct SFNTContainer **out)
{
  *out = NULL;
  enum EOTError result = constructContainer(out);
  struct SFNTOffsetTable offsetTable;
  enum StreamResult sResult = parseOffsetTable(streams[0], &offsetTable);
  if (sResult != EOT_STREAM_OK) {
    return EOT_CORRUPT_FILE;
  }
  result = reserveTables(*out, offsetTable.numTables);
  if (result != EOT_SUCCESS) {
    return result;
  }
  for (unsigned i = 0; i < offsetTable.numTables; ++i) {
    char tag[4];
    for (unsigned j = 0; j < 4; ++j) {
      RD2(BEReadChar, streams[0], tag + j, sResult);
    }
    struct SFNTTable *tbl;
    // TO-DO: Fix hdmx|VDMX table
    if (strncmp(tag, "hdmx", 4) == 0 || strncmp(tag, "VDMX", 4) == 0 ) {
      // skip checkSum, offset, length to next table offset
      sResult = seekRelative(streams[0], 12);
      logWarning("Ignoring hdmx/VDMX table -- will be fixed in a future release.\n");
      continue;
    }
    
    result = addTable(*out, tag, &tbl);
    /* skip the checksum, which we are not using for now. */
    sResult = seekRelative(streams[0], 4);
    if (sResult != EOT_STREAM_OK) {
      return EOT_CORRUPT_FILE;
    }
    RD2(BEReadU32, streams[0], &tbl->offset, sResult);
    RD2(BEReadU32, streams[0], &tbl->bufSize, sResult);
  }
  struct SFNTTable *glyf = NULL, *loca = NULL, *maxp = NULL, *head = NULL,
                   *hmtx = NULL, *hdmx = NULL, *VDMX = NULL;
  for (unsigned i = 0; i < (*out)->numTables; ++i) {
    struct SFNTTable *tbl = &((*out)->tables[i]);
    bool loadTable = true;
    if (strncmp(tbl->tag, "loca", 4) == 0) {
      loca = tbl;
      loadTable = false;
    } else if (strncmp(tbl->tag, "glyf", 4) == 0) {
      glyf = tbl;
      loadTable = false;
    } else if (strncmp(tbl->tag, "maxp", 4) == 0) {
      maxp = tbl;
    } else if (strncmp(tbl->tag, "head", 4) == 0) {
      head = tbl;
    } else if (strncmp(tbl->tag, "hmtx", 4) == 0) {
      hmtx = tbl;
    } else if (strncmp(tbl->tag, "hdmx", 4) == 0) {
      hdmx = tbl;
      loadTable = false;
    } else if (strncmp(tbl->tag, "cvt ", 4) == 0) {
      result = unpackCVT(tbl, streams[0]);
      if (result != EOT_SUCCESS) {
        return result;
      }
      loadTable = false;
    } else if (strncmp(tbl->tag, "VDMX", 4) == 0) {
      VDMX = tbl;
      loadTable = false;
    }
    if (loadTable) {
      result = loadTableFromStream(tbl, streams[0]);
      if (result != EOT_SUCCESS) {
        return result;
      }
      if (strncmp(tbl->tag, "head", 4) == 0) {
        /* kill global checksum; we will be recalcultaing it later. */
        if (tbl->bufSize < 12) {
          return EOT_MALFORMED_HEAD_TABLE;
        }
        for (unsigned i = 8; i < 12; ++i) {
          tbl->buf[i] = 0;
        }
      }
    }
  }
  if (!loca) {
    result = addTable(*out, "loca", &loca);
    if (result != EOT_SUCCESS) {
      return result;
    }
    if (!loca) {
      return EOT_LOGIC_ERROR;
    }
  }
  if (!maxp) {
    return EOT_NO_MAXP_TABLE;
  }
  if (!head) {
    return EOT_NO_HEAD_TABLE;
  }
  if (!hmtx) {
    return EOT_NO_HMTX_TABLE;
  }
  struct TTFheadData headData;
  result = TTFParseHead(head, &headData);
  if (result != EOT_SUCCESS) {
    return result;
  }
  struct TTFmaxpData maxpData;
  result = TTFParseMaxp(maxp, &maxpData);
  if (result != EOT_SUCCESS) {
    return result;
  }
  if (glyf) {
    result = populateGlyfAndLoca(glyf, loca, &headData, &maxpData, streams);
    if (result != EOT_SUCCESS) {
      return result;
    }
  }
  /* result = populateHdmx(hdmx, &maxpData, &headData, &hmtxData, streams[0]);
  if (result != EOT_SUCCESS)
  {
    return result;
  }
  */
  /* FIXME: Real hmdx and VMDX */
  if (hdmx) {
    hdmx->buf = NULL;
    hdmx->bufSize = 0;
    logWarning("Ignoring hdmx table -- will be fixed in a future release.\n");
  }
  if (VDMX) {
    VDMX->buf = NULL;
    VDMX->bufSize = 0;
    logWarning("Ignoring VDMX table -- will be fixed in a future release.\n");
  }
  return EOT_SUCCESS;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
