/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_STREAM_H__
#define __LIBEOT_STREAM_H__

#include <stdint.h>

enum StreamResult
{
  EOT_STREAM_OK,
  EOT_NOT_ENOUGH_DATA,
  EOT_NEGATIVE_SEEK,
  EOT_SEEK_PAST_EOS,
  EOT_CANT_ALLOCATE_MEMORY_FOR_STREAM,
  EOT_OUT_OF_RESERVED_SPACE,
  EOT_VALUE_OUT_OF_BOUNDS,
  EOT_OFF_BYTE_BOUNDARY
};

struct Stream
{
  uint8_t *buf;
  unsigned size;
  unsigned reserved;
  unsigned pos;
  unsigned bitPos;
};


struct Stream constructStream(uint8_t *buf, unsigned size);
struct Stream constructStream2(uint8_t *buf, unsigned size, unsigned reserved);
/* BE: Big Endian */
enum StreamResult BEReadU8(struct Stream *s, uint8_t *out);
enum StreamResult BEReadU16(struct Stream *s, uint16_t *out);
enum StreamResult BEReadU24(struct Stream *s, uint32_t *out);
enum StreamResult BEReadU32(struct Stream *s, uint32_t *out);

enum StreamResult BEReadS8(struct Stream *s, int8_t *out);
enum StreamResult BEReadS16(struct Stream *s, int16_t *out);
enum StreamResult BEReadS24(struct Stream *s, int32_t *out);
enum StreamResult BEReadS32(struct Stream *s, int32_t *out);

enum StreamResult BEReadChar(struct Stream *s, char *out);

enum StreamResult BEPeekU8(struct Stream *s, uint8_t *out);

enum StreamResult seekRelative(struct Stream *s, int offset);
enum StreamResult seekAbsolute(struct Stream *s, unsigned pos);

enum StreamResult seekRelativeThroughReserve(struct Stream *s, int offset);
enum StreamResult seekAbsoluteThroughReserve(struct Stream *s, unsigned pos);

enum StreamResult reserve(struct Stream *s, unsigned toReserve);

enum StreamResult BEWriteU8(struct Stream *s, uint8_t in);
enum StreamResult BEWriteU16(struct Stream *s, uint16_t in);
enum StreamResult BEWriteU24(struct Stream *s, uint32_t in);
enum StreamResult BEWriteU32(struct Stream *s, uint32_t in);

enum StreamResult BEReadRestAsU32(struct Stream *s, uint32_t *out);

enum StreamResult BEWriteS8(struct Stream *s, int8_t in);
enum StreamResult BEWriteS16(struct Stream *s, int16_t in);
enum StreamResult BEWriteS24(struct Stream *s, int32_t in);
enum StreamResult BEWriteS32(struct Stream *s, int32_t in);

enum StreamResult streamCopy(struct Stream *sIn, struct Stream *sOut, unsigned length);

enum StreamResult readNBits(struct Stream *s, uint32_t *out, unsigned n);

enum StreamResult BEcheckSum32(struct Stream *s, uint32_t *out, unsigned beginPos, unsigned endPos);

#define RD(fn, s, out, r) if ((r = fn(s, out)) != EOT_STREAM_OK) return r;
#define RD2(fn, s, out, r) if ((r = fn(s, out)) != EOT_STREAM_OK) return EOT_CORRUPT_FILE;
#define CHK_RD(sRes) if (sRes != EOT_STREAM_OK) return sRes
#define CHK_RD2(sRes) if (sRes != EOT_STREAM_OK) return EOT_CORRUPT_FILE
#define CHK_CN(sRes, fail) if (sRes != EOT_STREAM_OK) {returnedStatus = fail; goto CLEANUP;}


#endif /* #define __LIBEOT_STREAM_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
