/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_EOT_H__
#define __LIBEOT_EOT_H__

#include <stdbool.h>
#include <stdint.h>

#include "EOTError.h"

struct EUDCInfo
{
  bool exists;
  uint32_t codePage;
  uint32_t flags;
  uint32_t fontDataSize;
  uint8_t *fontData;
};

enum EOTVersion
{
  VERSION_1 = 1,
  VERSION_2 = 2,
  VERSION_3 = 3
};

enum EOTCharset
{
  ANSI_CHARSET = 0,
  DEFAULT_CHARSET = 1,
  SYMBOL_CHARSET = 2,
  MAC_CHARSET = 77,
  SHIFTJIS_CHARSET = 128,
  JOHAB_CHARSET = 130,
  HANGUL_CHARSET = 131,
  GB2312_CHARSET = 134,
  CHINESEBIG5_CHARSET = 136,
  GREEK_CHARSET = 161,
  TURKISH_CHARSET = 162,
  VIETNAMESE_CHARSET = 163,
  HEBREW_CHARSET = 177,
  ARABIC_CHARSET = 178,
  BALTIC_CHARSET = 186,
  RUSSIAN_CHARSET = 204,
  THAI_CHARSET = 222,
  EASTEUROPE_CHARSET = 238,
  OEM_CHARSET = 255
};

struct EOTRootStringInfo
{
  uint16_t rootStringSize;
  uint16_t *rootString;
};

struct EOTMetadata
{
  uint32_t totalSize;
  enum EOTVersion version;
  uint32_t flags;
  uint8_t panose[10];
  enum EOTCharset charset;
  bool italic;
  uint32_t weight;
  uint16_t permissions;
  uint32_t unicodeRange[4];
  uint32_t codePageRange[2];
  uint32_t checkSumAdjustment;
  uint16_t familyNameSize;
  uint16_t *familyName;
  uint16_t styleNameSize;
  uint16_t *styleName;
  uint16_t versionNameSize;
  uint16_t *versionName;
  uint16_t fullNameSize;
  uint16_t *fullName;
  unsigned numRootStrings;
  struct EOTRootStringInfo *rootStrings;
  uint32_t fontDataSize;
  unsigned fontDataOffset;
  struct EUDCInfo eudcInfo;
  /* used for storing the whole string so it can be
   * deleted properly if there is an error in the metadata parser. */
  uint16_t do_not_use_size;
  uint16_t *do_not_use;
};

unsigned EOTgetMetadataLength(const uint8_t *bytes);
enum EOTError EOTfillMetadata(const uint8_t *bytes, unsigned bytesLength,
    struct EOTMetadata *out);
void EOTfreeMetadata(struct EOTMetadata *toFree);
bool EOTcanLegallyEdit(const struct EOTMetadata *metadata);

#endif /* #define __LIBEOT_EOT_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
