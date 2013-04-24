/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#include <string.h>
#include <stdlib.h>

#include "EOT.h"

uint32_t EOTreadU32LE(uint8_t *bytes)
{
  return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

uint16_t EOTreadU16LE(uint8_t *bytes)
{
  return bytes[0] | (bytes[1] << 8);
}

unsigned EOTgetMetadataLength(uint8_t *bytes)
{
  uint32_t totalLength = EOTreadU32LE(bytes);
  uint32_t fontLength = EOTreadU32LE(bytes + 4);
  return totalLength - fontLength;
}

enum EOTError EOTgetString(uint8_t **scanner, uint8_t *begin, unsigned bytesLength,
      uint16_t *size, uint16_t **string)
{
  if (*string)
  {
    free(*string);
  }
  *string = 0;
  if (*scanner - begin + 2 >= bytesLength)
  {
    return EOT_INSUFFICIENT_BYTES;
  }
  *size = EOTreadU16LE(*scanner);
  *scanner += 2;
  if (*size % 2 != 0) // strings are encoded in UTF-16.
  {
    return EOT_BOGUS_STRING_SIZE;
  }
  if (*scanner - begin + *size >= bytesLength)
  {
    return EOT_INSUFFICIENT_BYTES;
  }
  if (*size != 0)
  {
    *string = malloc(*size);
    if (!*string)
    {
      return EOT_CANT_ALLOCATE_MEMORY;
    }
    for (unsigned i = 0; i < *size / 2; ++i)
    {
      (*string)[i] = EOTreadU16LE(*scanner);
      *scanner += 2;
    }
  }
  return EOT_SUCCESS;
}

enum EOTError EOTgetByteArray(uint8_t **scanner, uint8_t *begin,
    unsigned bytesLength, uint32_t *size, uint8_t **array)
{
  if (*array)
  {
    free(*array);
  }
  *array = 0;
  if (*scanner - begin + 4 >= bytesLength)
  {
    return EOT_INSUFFICIENT_BYTES;
  }
  *size = EOTreadU32LE(*scanner);
  *scanner += 4;
  if (*scanner - begin + *size >= bytesLength)
  {
    return EOT_INSUFFICIENT_BYTES;
  }
  if (*size != 0)
  {
    *array = malloc(*size);
    if (!*array)
    {
      return EOT_CANT_ALLOCATE_MEMORY;
    }
    for (unsigned i = 0; i < *size; ++i)
    {
      (*array)[i] = **scanner;
      ++(*scanner);
    }
  }
  return EOT_SUCCESS;
}

void EOTfreeMetadata(struct EOTMetadata *d)
{
  if (d->familyName)
  {
    free(d->familyName);
  }
  if (d->styleName)
  {
    free(d->styleName);
  }
  if (d->versionName)
  {
    free(d->versionName);
  }
  if (d->fullName)
  {
    free(d->fullName);
  }
  if (d->do_not_use)
  {
    free(d->do_not_use);
  }
  if (d->rootStrings)
  {
    for (unsigned i = 0; i < d->numRootStrings; ++i)
    {
      free(d->rootStrings[i].rootString);
    }
    free(d->rootStrings);
  }
  if (d->eudcInfo.fontData)
  {
    free(d->eudcInfo.fontData);
  }
}

#define EOT_ENSURE_SCANNER(N) if (scanner - bytes + N >= bytesLength) { EOTfreeMetadata(out); return EOT_INSUFFICIENT_BYTES; }

#define EOT_ENSURE_STRING_NOERR(E) {enum EOTError macro_defined_var_E = E; if(macro_defined_var_E != EOT_SUCCESS) { EOTfreeMetadata(out); return macro_defined_var_E; }}

enum EOTError EOTfillMetadata(uint8_t *bytes, unsigned bytesLength,
    struct EOTMetadata *out)
{
  struct EOTMetadata zero = {0};
  if (bytesLength < 8 || bytesLength < EOTgetMetadataLength(bytes))
  {
    return EOT_INSUFFICIENT_BYTES;
  }
  *out = zero;
  uint8_t *scanner = bytes;
  EOT_ENSURE_SCANNER(4);
  out->totalSize = EOTreadU32LE(scanner);
  scanner += 4;
  EOT_ENSURE_SCANNER(4);
  out->fontDataSize = EOTreadU32LE(scanner);
  scanner += 4;
  EOT_ENSURE_SCANNER(4);
  uint32_t versionMagic = EOTreadU32LE(scanner);
  scanner += 4;
  switch (versionMagic)
  {
  case 0x00010000:
    out->version = VERSION_1;
    break;
  case 0x00020001:
    out->version = VERSION_2;
    break;
  case 0x00020002:
    out->version = VERSION_3;
    break;
  default:
    return EOT_CORRUPT_FILE;
  }
  EOT_ENSURE_SCANNER(4);
  out->flags = EOTreadU32LE(scanner);
  scanner += 4;
  EOT_ENSURE_SCANNER(10);
  memcpy(&(out->panose), scanner, 10);
  scanner += 10;
  EOT_ENSURE_SCANNER(1);
  out->charset = (enum EOTCharset)(*scanner);
  ++scanner;
  EOT_ENSURE_SCANNER(1);
  out->italic = *scanner;
  ++scanner;
  EOT_ENSURE_SCANNER(4);
  out->weight = EOTreadU32LE(scanner);
  scanner += 4;
  EOT_ENSURE_SCANNER(2);
  out->permissions = EOTreadU16LE(scanner);
  scanner += 2;
  EOT_ENSURE_SCANNER(2);
  if (EOTreadU16LE(scanner) != 0x504C)
  {
    return EOT_CORRUPT_FILE;
  }
  scanner += 2;
  for (unsigned i = 0; i < 4; ++i)
  {
    EOT_ENSURE_SCANNER(4);
    out->unicodeRange[i] = EOTreadU32LE(scanner);
    scanner += 4;
  }
  for (unsigned i = 0; i < 2; ++i)
  {
    EOT_ENSURE_SCANNER(4);
    out->codePageRange[i] = EOTreadU32LE(scanner);
    scanner += 4;
  }
  EOT_ENSURE_SCANNER(4);
  out->checkSumAdjustment = EOTreadU32LE(scanner);
  scanner += 22;
  EOT_ENSURE_STRING_NOERR(EOTgetString(&scanner, bytes, bytesLength, &(out->familyNameSize),
        &(out->familyName)));
  scanner += 2;
  EOT_ENSURE_STRING_NOERR(EOTgetString(&scanner, bytes, bytesLength, &(out->styleNameSize),
        &(out->styleName)));
  scanner += 2;
  EOT_ENSURE_STRING_NOERR(EOTgetString(&scanner, bytes, bytesLength, &(out->versionNameSize),
        &(out->versionName)));
  scanner += 2;
  EOT_ENSURE_STRING_NOERR(EOTgetString(&scanner, bytes, bytesLength, &(out->fullNameSize),
        &(out->fullName)));
  if (out->version > VERSION_1)
  {
    scanner += 2;
    EOT_ENSURE_STRING_NOERR(EOTgetString(&scanner, bytes, bytesLength, &(out->do_not_use_size),
          &(out->do_not_use)));
    EOT_ENSURE_SCANNER(4);
    EOTreadU32LE(scanner); /* root string checksum */
    scanner += 4;
    if (out->version == VERSION_3)
    {
      EOT_ENSURE_SCANNER(4);
      out->eudcInfo.codePage = EOTreadU32LE(scanner);
      scanner += 6;
      EOT_ENSURE_SCANNER(2);
      uint16_t signatureSize = EOTreadU16LE(scanner);
      scanner += 2;
      EOT_ENSURE_SCANNER(signatureSize);
      scanner += signatureSize;
      /* signature is reserved, so do nothing with this. */
      EOT_ENSURE_SCANNER(4);
      out->eudcInfo.flags = EOTreadU32LE(scanner);
      scanner += 4;
      EOT_ENSURE_STRING_NOERR(EOTgetByteArray(&scanner, bytes, bytesLength,
            &(out->eudcInfo.fontDataSize), &(out->eudcInfo.fontData)));
      if (out->eudcInfo.fontDataSize > 0)
      {
        out->eudcInfo.exists = true;
      }
    }
  }
  out->fontDataOffset = scanner - bytes;
  return EOT_SUCCESS;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
