/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#include <string.h>
#include <stdlib.h>

#include <libeot/libeot.h>

const uint16_t EDITING_MASK = 0x0008;

uint32_t EOTreadU32LE(const uint8_t *bytes)
{
  return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

uint16_t EOTreadU16LE(const uint8_t *bytes)
{
  return bytes[0] | (bytes[1] << 8);
}

unsigned EOTgetMetadataLength(const uint8_t *bytes)
{
  uint32_t totalLength = EOTreadU32LE(bytes);
  uint32_t fontLength = EOTreadU32LE(bytes + 4);
  return totalLength - fontLength;
}

enum EOTError EOTgetString(const uint8_t **scanner, const uint8_t *begin, unsigned bytesLength,
      uint16_t *size, uint16_t **string)
{
  if (*string)
  {
    free(*string);
  }
  *string = 0;
  if (*scanner - begin + 2 > bytesLength)
  {
    return EOT_INSUFFICIENT_BYTES;
  }
  *size = EOTreadU16LE(*scanner);
  *scanner += 2;
  if (*size % 2 != 0) // strings are encoded in UTF-16.
  {
    return EOT_BOGUS_STRING_SIZE;
  }
  if (*scanner - begin + *size > bytesLength)
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

enum EOTError EOTgetByteArray(const uint8_t **scanner, const uint8_t *begin,
    unsigned bytesLength, uint32_t *size, uint8_t **array)
{
  if (*array)
  {
    free(*array);
  }
  *array = 0;
  if (*scanner - begin + 4 > bytesLength)
  {
    return EOT_INSUFFICIENT_BYTES;
  }
  *size = EOTreadU32LE(*scanner);
  *scanner += 4;
  if (*scanner - begin + *size > bytesLength)
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
  struct EOTMetadata zero = {0};
  *d = zero;
}

#define EOT_ENSURE_SCANNER(N) if (scanner - bytes + N >= bytesLength) { EOTfreeMetadata(out); return EOT_INSUFFICIENT_BYTES; }

#define EOT_ENSURE_STRING_NOERR(E) {enum EOTError macro_defined_var_E = E; if(macro_defined_var_E != EOT_SUCCESS) { EOTfreeMetadata(out); return macro_defined_var_E; }}

enum EOTError EOTfillMetadataSpecifyingVersion(const uint8_t *bytes, unsigned bytesLength,
    struct EOTMetadata *out, enum EOTVersion version, int currIndex)
{
  out->version = version;
  const uint8_t *scanner = bytes;
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
    if (out->version == VERSION_3)
    {
      EOT_ENSURE_SCANNER(4);
      EOTreadU32LE(scanner); /* root string checksum */
      scanner += 4;
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
  out->fontDataOffset = scanner - bytes + currIndex;
  int expectedHeaderSize = (int)(out->totalSize) - (int)(out->fontDataSize);
  if (out->fontDataOffset < expectedHeaderSize)
  {
    return EOT_HEADER_TOO_BIG;
  }
  return EOT_SUCCESS;
}

enum EOTError EOTfillMetadata(const uint8_t *bytes, unsigned bytesLength,
    struct EOTMetadata *out)
{
  struct EOTMetadata zero = {0};
  *out = zero;
  const uint8_t *scanner = bytes;
  if (bytesLength < 8 || bytesLength < EOTgetMetadataLength(bytes))
  {
    return EOT_INSUFFICIENT_BYTES;
  }
  EOT_ENSURE_SCANNER(4);
  unsigned totalSize = EOTreadU32LE(scanner);
  scanner += 4;
  EOT_ENSURE_SCANNER(4);
  unsigned fontDataSize = EOTreadU32LE(scanner);
  scanner += 4;
  EOT_ENSURE_SCANNER(4);
  uint32_t versionMagic = EOTreadU32LE(scanner);
  scanner += 4;
  enum EOTVersion codedVersion;
  switch (versionMagic)
  {
  case 0x00010000:
    codedVersion = VERSION_1;
    break;
  case 0x00020001:
    codedVersion = VERSION_2;
    break;
  case 0x00020002:
    codedVersion = VERSION_3;
    break;
  default:
    return EOT_CORRUPT_FILE;
  }
  enum EOTVersion tryVersion = codedVersion;
  bool bumpedUp = false, knockedDown = false;
  while (true)
  {
    EOTfreeMetadata(out);
    out->totalSize = totalSize;
    out->fontDataSize = fontDataSize;
    if (bytesLength + bytes < out->fontDataSize + scanner)
    {
      return EOT_CORRUPT_FILE;
    }
    enum EOTError result = EOTfillMetadataSpecifyingVersion(scanner, bytesLength - out->fontDataSize - (scanner - bytes), out, tryVersion, scanner - bytes);
    if (result == EOT_SUCCESS)
    {
      return tryVersion == codedVersion ? EOT_SUCCESS : EOT_WARN_BAD_VERSION;
    }
    if (result == EOT_HEADER_TOO_BIG)
    {
      if (knockedDown || (tryVersion == VERSION_3))
      {
        return EOT_CORRUPT_FILE;
      }
      knockedDown = false;
      bumpedUp = true;
      ++tryVersion;
    }
    else if (result == EOT_INSUFFICIENT_BYTES)
    {
      if (bumpedUp || (tryVersion == VERSION_1))
      {
        return EOT_CORRUPT_FILE;
      }
      knockedDown = true;
      bumpedUp = false;
      --tryVersion;
    }
    else
    {
      return result;
    }
  }
}

/* Please think twice before circumventing this function.
 * Does your personal sense of morality really let you take others' work without their permission?
 *
 * I'm not suggesting any system of morality is right or wrong;
 * I'm merely asking that you reflect on it before changing anything here.
 */
bool EOTcanLegallyEdit(const struct EOTMetadata *metadata)
{
  return metadata->permissions == 0 || ((metadata->permissions & EDITING_MASK) != 0);
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
