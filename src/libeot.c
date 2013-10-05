/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <err.h>

#include <sys/stat.h>

#include "EOT.h"
#include "flags.h"
#include "writeFontFile.h"

void printError(enum EOTError error, FILE *out)
{
  switch (error)
  {
  case EOT_SUCCESS:
    break;
  case EOT_INSUFFICIENT_BYTES:
    fputs("The font file appears truncated.\n", out);
    break;
  case EOT_BOGUS_STRING_SIZE:
  case EOT_CORRUPT_FILE:
    fputs("The font file appears corrupt.\n", out);
    break;
  case EOT_CANT_ALLOCATE_MEMORY:
    fputs("Couldn't allocate sufficient memory.\n", out);
    break;
  case EOT_OTHER_STDLIB_ERROR:
    fputs("There was an unknown system error.\n", out);
    break;
  case EOT_COMPRESSION_NOT_YET_IMPLEMENTED:
    fputs("MTX Compression has not yet been implemented in this version of libeot. The font could therefore not be converted.\n", out);
    break;
  default:
    fputs("Unknown error: this is a bug in libeot; it does not *necessarily* indicate a corrupted font file.\n", out);
    break;
  }

}

enum EOTError eot2ttf_file(const uint8_t *font, unsigned fontSize, struct EOTMetadata *metadataOut, FILE *out)
{
  enum EOTError result = EOTfillMetadata(font, fontSize, metadataOut);
  if (result >= EOT_WARN)
  {
    printError(result, stderr);
  }
  else if (result != EOT_SUCCESS)
  {
    return result;
  }

  enum EOTError writeResult = writeFontFile(font + metadataOut->fontDataOffset, metadataOut->fontDataSize, metadataOut->flags & TTEMBED_TTCOMPRESSED, metadataOut->flags & TTEMBED_XORENCRYPTDATA, out);
  if (writeResult != EOT_SUCCESS)
  {
    return writeResult;
  }
  return EOT_SUCCESS;
}

enum EOTError eot2ttf_buffer(const uint8_t *font, unsigned fontSize, struct EOTMetadata *metadataOut, uint8_t **fontOut,
    unsigned *fontSizeOut)
{
  enum EOTError result = EOTfillMetadata(font, fontSize, metadataOut);
  if (result >= EOT_WARN)
  {
    printError(result, stderr);
  }
  else if (result != EOT_SUCCESS)
  {
    return result;
  }

  enum EOTError writeResult = writeFontBuffer(font + metadataOut->fontDataOffset, metadataOut->fontDataSize, metadataOut->flags & TTEMBED_TTCOMPRESSED, metadataOut->flags & TTEMBED_XORENCRYPTDATA, fontOut, fontSizeOut);
  if (result >= EOT_WARN)
  {
    printError(result, stderr);
  }
  else if (writeResult != EOT_SUCCESS)
  {
    return writeResult;
  }
  return EOT_SUCCESS;
}

void freeEOTBuffer(const uint8_t *buffer)
{
  free(buffer);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
