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

void printError(enum EOTError error, const char *filename, FILE *out)
{
  switch (error)
  {
  case EOT_SUCCESS:
    break;
  case EOT_INSUFFICIENT_BYTES:
    fprintf(out, "The file %s appears truncated.\n", filename);
    break;
  case EOT_BOGUS_STRING_SIZE:
  case EOT_CORRUPT_FILE:
    fprintf(out, "The file %s appears corrupt.\n", filename);
    break;
  case EOT_CANT_ALLOCATE_MEMORY:
    fprintf(out, "Couldn't allocate sufficient memory.\n");
    break;
  case EOT_OTHER_STDLIB_ERROR:
    fprintf(out, "There was an unknown system error.\n");
    break;
  case EOT_COMPRESSION_NOT_YET_IMPLEMENTED:
    fprintf(out, "MTX Compression has not yet been implemented in this version of libeot. The font could therefore not be converted.\n");
    break;
  default:
    fprintf(out, "Unknown error: this is a bug in libeot; it does not *necessarily* indicate a corrupted font file.\n");
    break;
  }

}

enum EOTError eot2ttf_file(const uint8_t *font, unsigned fontSize, struct EOTMetadata *metadataOut, FILE *out)
{
  enum EOTError result = EOTfillMetadata(font, fontSize, metadataOut);
  if (result != EOT_SUCCESS)
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
  if (result != EOT_SUCCESS)
  {
    return result;
  }

  enum EOTError writeResult = writeFontBuffer(font + metadataOut->fontDataOffset, metadataOut->fontDataSize, metadataOut->flags & TTEMBED_TTCOMPRESSED, metadataOut->flags & TTEMBED_XORENCRYPTDATA, fontOut, fontSizeOut);
  if (writeResult != EOT_SUCCESS)
  {
    return writeResult;
  }
  return EOT_SUCCESS;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
