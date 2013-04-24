/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "stream.h"

enum StreamResult BEReadRestAsU32(struct Stream *s, uint32_t *out)
{
  if (s->pos >= s->size)
  {
    return EOT_NOT_ENOUGH_DATA;
  }
  uint8_t o8;
  uint16_t o16;
  switch (s->size - s->pos)
  {
  case 1:
    BEReadU8(s, &o8);
    *out = ((uint32_t)o8) << 24;
    break;
  case 2:
    BEReadU16(s, &o16);
    *out = ((uint32_t)o16) << 16;
    break;
  case 3:
    BEReadU24(s, out);
    *out <<= 8;
    break;
  case 4:
  default:
    BEReadU32(s, out);
    break;
  }
  return EOT_STREAM_OK;
}

struct Stream constructStream(uint8_t *buf, unsigned size)
{
  return constructStream2(buf, size, size);
}

struct Stream constructStream2(uint8_t *buf, unsigned size, unsigned reserved)
{
  struct Stream ret;
  ret.buf = buf;
  ret.size = size;
  ret.pos = 0;
  ret.reserved = reserved;
  ret.bitPos = 0;
  return ret;
}

enum StreamResult BEReadU8(struct Stream *s, uint8_t *out)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  if (s->pos >= s->size)
  {
    return EOT_NOT_ENOUGH_DATA;
  }
  *out = s->buf[s->pos++];
  return EOT_STREAM_OK;
}

enum StreamResult BEReadChar(struct Stream *s, char *out)
{
  return BEReadU8(s, (uint8_t *)out);
}

enum StreamResult BEReadU16(struct Stream *s, uint16_t *out)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  if (s->pos + 2 > s->size)
  {
    return EOT_NOT_ENOUGH_DATA;
  }
  *out = (((uint16_t)(s->buf[s->pos])) << 8) | ((uint16_t)(s->buf[s->pos + 1]));
  s->pos += 2;
  return EOT_STREAM_OK;
}

enum StreamResult BEReadU24(struct Stream *s, uint32_t *out)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  if (s->pos + 3 > s->size)
  {
    return EOT_NOT_ENOUGH_DATA;
  }
  *out = (((uint32_t)(s->buf[s->pos])) << 16) | (((uint32_t)(s->buf[s->pos + 1])) << 8) | ((uint32_t)(s->buf[s->pos + 2]));
  s->pos += 3;
  return EOT_STREAM_OK;
}

enum StreamResult BEReadU32(struct Stream *s, uint32_t *out)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  if (s->pos + 4 > s->size)
  {
    return EOT_NOT_ENOUGH_DATA;
  }
  *out = (((uint32_t)(s->buf[s->pos])) << 24) | (((uint32_t)(s->buf[s->pos + 1])) << 16) | (((uint32_t)(s->buf[s->pos + 2])) << 8) | ((uint32_t)(s->buf[s->pos + 3]));
  s->pos += 4;
  return EOT_STREAM_OK;
}

enum StreamResult BEReadS8(struct Stream *s, int8_t *out)
{
  return BEReadU8(s, (uint8_t *)out);
}

enum StreamResult BEReadS16(struct Stream *s, int16_t *out)
{
  return BEReadU16(s, (uint16_t *)out);
}

enum StreamResult BEReadS24(struct Stream *s, int32_t *out)
{
  return BEReadU24(s, (uint32_t *)out);
}

enum StreamResult BEReadS32(struct Stream *s, int32_t *out)
{
  return BEReadU32(s, (uint32_t *)out);
}

enum StreamResult BEPeekU8(struct Stream *s, uint8_t *out)
{
  enum StreamResult ret1 = BEReadU8(s, out);
  --s->pos;
  return ret1;
}

enum StreamResult seekRelative(struct Stream *s, int offset)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  int newPos = s->pos + offset;
  if (newPos < 0)
  {
    return EOT_NEGATIVE_SEEK;
  }
  if (newPos > s->size)
  {
    return EOT_SEEK_PAST_EOS;
  }
  s->pos = (unsigned)newPos;
  return EOT_STREAM_OK;
}

enum StreamResult seekRelativeThroughReserve(struct Stream *s, int offset)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  int newPos = s->pos + offset;
  if (newPos < 0)
  {
    return EOT_NEGATIVE_SEEK;
  }
  if (newPos > s->reserved)
  {
    return EOT_SEEK_PAST_EOS;
  }
  s->pos = (unsigned)newPos;
  return EOT_STREAM_OK;
}

enum StreamResult seekAbsolute(struct Stream *s, unsigned pos)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  if (pos > s->size)
  {
    return EOT_SEEK_PAST_EOS;
  }
  s->pos = pos;
  return EOT_STREAM_OK;
}

enum StreamResult seekAbsoluteThroughReserve(struct Stream *s, unsigned pos)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  if (pos > s->reserved)
  {
    return EOT_SEEK_PAST_EOS;
  }
  s->pos = pos;
  return EOT_STREAM_OK;
}

enum StreamResult reserve(struct Stream *s, unsigned toReserve)
{
  if (s->reserved >= toReserve)
  {
    return EOT_STREAM_OK;
  }
  uint8_t *newBuf = realloc(s->buf, toReserve);
  if (!newBuf)
  {
    return EOT_CANT_ALLOCATE_MEMORY_FOR_STREAM;
  }
  s->buf = newBuf;
  s->reserved = toReserve;
  return EOT_STREAM_OK;
}
#define CHK_RES(s, n) if (s->pos + n > s->reserved) return EOT_OUT_OF_RESERVED_SPACE
#define FIX_SIZE(s) if (s->pos > s->size) s->size = s->pos
enum StreamResult BEWriteU8(struct Stream *s, uint8_t in)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  CHK_RES(s, 1);
  s->buf[s->pos++] = in;
  FIX_SIZE(s);
  return EOT_STREAM_OK;
}
enum StreamResult BEWriteU16(struct Stream *s, uint16_t in)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  CHK_RES(s, 2);
  s->buf[s->pos++] = (uint8_t)(in >> 8);
  s->buf[s->pos++] = (uint8_t)(in & 0xFF);
  FIX_SIZE(s);
  return EOT_STREAM_OK;
}
enum StreamResult BEWriteU24(struct Stream *s, uint32_t in)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  if (in & 0xFF000000)
  {
    return EOT_VALUE_OUT_OF_BOUNDS;
  }
  CHK_RES(s, 3);
  s->buf[s->pos++] = (uint8_t)(in >> 16);
  s->buf[s->pos++] = (uint8_t)((in >> 8) & 0xFF);
  s->buf[s->pos++] = (uint8_t)(in & 0xFF);
  FIX_SIZE(s);
  return EOT_STREAM_OK;
}
enum StreamResult BEWriteU32(struct Stream *s, uint32_t in)
{
  if (s->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  CHK_RES(s, 4);
  s->buf[s->pos++] = (uint8_t)(in >> 24);
  s->buf[s->pos++] = (uint8_t)((in >> 16) & 0xFF);
  s->buf[s->pos++] = (uint8_t)((in >> 8) & 0xFF);
  s->buf[s->pos++] = (uint8_t)(in & 0xFF);
  FIX_SIZE(s);
  return EOT_STREAM_OK;
}
enum StreamResult BEWriteS16(struct Stream *s, int16_t in)
{
  return BEWriteU16(s, (uint16_t)in);
}

enum StreamResult streamCopy(struct Stream *sIn, struct Stream *sOut, unsigned length)
{
  if (sIn->bitPos != 0 || sOut->bitPos != 0)
  {
    return EOT_OFF_BYTE_BOUNDARY;
  }
  if (sIn->pos + length > sIn->size)
  {
    return EOT_NOT_ENOUGH_DATA;
  }
  if (sOut->pos + length > sOut->reserved)
  {
    return EOT_OUT_OF_RESERVED_SPACE;
  }
  memcpy(sOut->buf + sOut->pos, sIn->buf + sIn->pos, length);
  sOut->pos += length;
  sIn->pos += length;
  return EOT_STREAM_OK;
}

/* FIXME: This could be made A LOT faster. I am too lazy to figure out how. */
enum StreamResult readNBits(struct Stream *s, uint32_t *out, unsigned n)
{
  const uint8_t masks[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
  if (n > 32)
  {
    return EOT_VALUE_OUT_OF_BOUNDS;
  }
  *out = 0;
  for (unsigned i = 0; i < n; ++i)
  {
    if (s->pos >= s->size)
    {
      return EOT_NOT_ENOUGH_DATA;
    }
    bool bitSet = (s->buf[s->pos] & masks[s->bitPos]) > 0;
    *out |= ( bitSet ? 1 : 0 ) << (n - i - 1);
    ++s->bitPos;
    if (s->bitPos == 8)
    {
      s->bitPos = 0;
      ++s->pos;
    }
  }
  return EOT_STREAM_OK;
}

enum StreamResult BEcheckSum32(struct Stream *s, uint32_t *out, unsigned beginPos, unsigned endPos)
{
  if (beginPos > endPos)
  {
    return EOT_VALUE_OUT_OF_BOUNDS;
  }
  if (endPos > s->size)
  {
    return EOT_NOT_ENOUGH_DATA;
  }
  struct Stream slice = constructStream(s->buf + beginPos, endPos - beginPos);
  enum StreamResult sResult = EOT_STREAM_OK;
  *out = 0;
  while (sResult == EOT_STREAM_OK)
  {
    uint32_t chunk;
    sResult = BEReadRestAsU32(&slice, &chunk);
    *out += chunk;
  }
  if (sResult == EOT_NOT_ENOUGH_DATA)
  {
    sResult = EOT_STREAM_OK;
  }
  return sResult;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
