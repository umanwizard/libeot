/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <libeot/libeot.h>

#include "../util/stream.h"
#include "SFNTContainer.h"

enum EOTError reserveTables(struct SFNTContainer *ctr, unsigned num)
{
  if (ctr->_numTablesReserved >= num)
  {
    return EOT_SUCCESS;
  }
  void *allocated = realloc(ctr->tables, sizeof(struct SFNTTable) * num);
  if (!allocated)
  {
    return EOT_CANT_ALLOCATE_MEMORY;
  }
  ctr->tables = (struct SFNTTable *)allocated;
  ctr->_numTablesReserved = num;
  return EOT_SUCCESS;
}

enum EOTError constructContainer(struct SFNTContainer **out)
{
  *out = (struct SFNTContainer *)malloc(sizeof(struct SFNTContainer));
  if (!out)
  {
    return EOT_CANT_ALLOCATE_MEMORY;
  }
  (*out)->numTables = 0;
  (*out)->_numTablesReserved = 0;
  (*out)->tables = NULL;
  return EOT_SUCCESS;
}

void _freeTable(struct SFNTTable *tbl)
{
  free(tbl->buf);
  tbl->buf = NULL;
}

void freeContainer(struct SFNTContainer *ctr)
{
  for (unsigned i = 0; i < ctr->numTables; ++i)
  {
    _freeTable(ctr->tables + i);
  }
  free(ctr->tables);
  free(ctr);
}

enum StreamResult _writeTblCheckingSum(struct SFNTTable *tbl, struct Stream *out)
{
  tbl->checksum = 0;
  tbl->offset = out->pos;
  struct Stream tblStream = constructStream(tbl->buf, tbl->bufSize);
  enum StreamResult sResult = EOT_STREAM_OK;
  enum StreamResult sResult2;
  while (true)
  {
    uint32_t chunk;
    sResult = BEReadRestAsU32(&tblStream, &chunk);
    if (sResult == EOT_STREAM_OK)
    {
      tbl->checksum += chunk;
      sResult2 = BEWriteU32(out, chunk);
      CHK_RD(sResult2);
    }
    else
    {
      break;
    }
  }
  if (sResult == EOT_NOT_ENOUGH_DATA)
  {
    return EOT_STREAM_OK;
  }
  return sResult;
}

enum StreamResult _writeTableDirectory(struct SFNTContainer *ctr, struct Stream *out)
{
  enum StreamResult sResult;
  for (unsigned i = 0; i < ctr->numTables; ++i)
  {
    struct SFNTTable *tbl = &ctr->tables[i];
    for (unsigned iTag = 0; iTag < 4; ++iTag)
    {
      RD(BEWriteU8, out, (char)(tbl->tag[iTag]), sResult);
    }
    RD(BEWriteU32, out, tbl->checksum, sResult);
    RD(BEWriteU32, out, tbl->offset, sResult);
    RD(BEWriteU32, out, tbl->bufSize, sResult);
  }
  return EOT_STREAM_OK;
}

/* log_2(largest power of 2 <= n) */
unsigned _lgflr(unsigned n)
{
  unsigned ret = 0;
  while (n > 1)
  {
    n /= 2;
    ++ret;
  }
  return ret;
}

/* largest power of 2 <= n */
unsigned _maxpw(unsigned n)
{
  unsigned ret = 1;
  while (n > 1)
  {
    ret *= 2;
    n /= 2;
  }
  return ret;
}

enum StreamResult _writeOffsetTable(struct SFNTContainer *ctr, struct Stream *out)
{
  enum StreamResult sResult;
  uint32_t scalerType = 0x00010000; /* magical number from TTF standard */
  uint16_t numTables = (uint16_t)(ctr->numTables);
  uint16_t searchRange = (uint16_t)(_maxpw(ctr->numTables) * 16);
  uint16_t entrySelector = (uint16_t)(_lgflr(ctr->numTables));
  uint16_t rangeShift = numTables * 16 - searchRange;
  RD(BEWriteU32, out, scalerType, sResult);
  RD(BEWriteU16, out, numTables, sResult);
  RD(BEWriteU16, out, searchRange, sResult);
  RD(BEWriteU16, out, entrySelector, sResult);
  RD(BEWriteU16, out, rangeShift, sResult);
  return EOT_STREAM_OK;
}

unsigned _getTableDirectorySize(struct SFNTContainer *ctr)
{
  return 16 * ctr->numTables;
}

unsigned _getRequiredSize(struct SFNTContainer *ctr)
{
  unsigned ret = 12; /* for offset table */
  ret += _getTableDirectorySize(ctr);
  for (unsigned i = 0; i < ctr->numTables; ++i)
  {
    struct SFNTTable *tbl = &ctr->tables[i];
    ret += ((tbl->bufSize + 3) / 4) * 4; /* pads out to 4-byte boundary */
  }
  return ret;
}



enum EOTError dumpContainer(struct SFNTContainer *ctr, uint8_t **outBuf, unsigned *outSize)
{
  struct Stream s = constructStream(NULL, 0);
  unsigned requiredSize = _getRequiredSize(ctr);
  enum StreamResult sResult = reserve(&s, requiredSize);
  if (sResult != EOT_STREAM_OK)
  {
    return EOT_CANT_ALLOCATE_MEMORY;
  }
  enum EOTError returnedStatus = EOT_SUCCESS;
  sResult = _writeOffsetTable(ctr, &s);
  CHK_CN(sResult, EOT_LOGIC_ERROR);
  unsigned tableDirectoryOffset = s.pos;
  sResult = seekRelativeThroughReserve(&s, _getTableDirectorySize(ctr));
  CHK_CN(sResult, EOT_LOGIC_ERROR);
  struct SFNTTable *head = NULL;
  unsigned chk = 0;
  for (unsigned i = 0; i < ctr->numTables; ++i)
  {
    struct SFNTTable *tbl = &ctr->tables[i];
    if (strncmp(tbl->tag, "head", 4) == 0)
    {
      head = tbl;
    }
    tbl->offset = s.pos;
    sResult = _writeTblCheckingSum(tbl, &s);
    CHK_CN(sResult, EOT_LOGIC_ERROR);
    chk += tbl->checksum;
  }
  if (head == NULL)
  {
    returnedStatus = EOT_LOGIC_ERROR; /* should have already caught the lack of a head table! */
    goto CLEANUP;
  }
  seekAbsolute(&s, tableDirectoryOffset);
  sResult = _writeTableDirectory(ctr, &s);
  CHK_CN(sResult, EOT_LOGIC_ERROR);
  unsigned beginningChk;
  sResult = BEcheckSum32(&s, &beginningChk, 0, s.pos);
  CHK_CN(sResult, EOT_LOGIC_ERROR);
  chk += beginningChk;
  /* now put in the global checksum. It's OK that this will make the head checksum incorrect! */
  /* this mystical number 0xB1B0AFBA is defined by the TTF standard, dunno why they picked this value. */
  unsigned finalChecksum = 0xB1B0AFBA - chk;
  struct Stream sChkOut = constructStream(head->buf, head->bufSize);
  sResult = seekAbsolute(&sChkOut, 8);
  CHK_CN(sResult, EOT_LOGIC_ERROR);
  sResult = BEWriteU32(&sChkOut, finalChecksum);
  CHK_CN(sResult, EOT_LOGIC_ERROR);
  returnedStatus = EOT_SUCCESS;
  *outBuf = s.buf;
  *outSize = s.size;
CLEANUP:
  /* lmao */
  return returnedStatus;
}

enum EOTError addTable(struct SFNTContainer *ctr, const char *tag, struct SFNTTable **newTableOut)
{
  if (ctr->numTables == ctr->_numTablesReserved)
  {
    enum EOTError err = reserveTables(ctr, ctr->numTables * 2);
    if (err != EOT_SUCCESS)
    {
      return err;
    }
  }
  struct SFNTTable *tbl = &ctr->tables[ctr->numTables++];
  for (unsigned i = 0; i < 4; ++i)
  {
    tbl->tag[i] = tag[i];
  }
  tbl->buf = NULL;
  tbl->bufSize = 0;
  tbl->offset = 0;
  *newTableOut = tbl;
  return EOT_SUCCESS;
}

#define CHK_RD2(sRes) if (sRes != EOT_STREAM_OK) return EOT_CORRUPT_FILE
enum EOTError loadTableFromStream(struct SFNTTable *tbl, struct Stream *s)
{
  enum StreamResult sResult = seekAbsolute(s, tbl->offset);
  CHK_RD2(sResult);
  tbl->buf = (uint8_t *)malloc(tbl->bufSize);
  struct Stream sOut = constructStream2(tbl->buf, 0, tbl->bufSize);
  sResult = streamCopy(s, &sOut, tbl->bufSize);
  CHK_RD2(sResult);
  return EOT_SUCCESS;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
