/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <err.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "EOT.h"
#include "flags.h"
#include "writeFontFile.h"

void usage(char *progName)
{
  fprintf(stderr, "Usage: %s myfont.eot out.ttf\n", progName);
}

void printError(enum EOTError error, const char *filename)
{
  switch (error)
  {
  case EOT_SUCCESS:
    break;
  case EOT_INSUFFICIENT_BYTES:
    fprintf(stderr, "The file %s appears truncated.\n", filename);
    break;
  case EOT_BOGUS_STRING_SIZE:
  case EOT_CORRUPT_FILE:
    fprintf(stderr, "The file %s appears corrupt.\n", filename);
    break;
  case EOT_CANT_ALLOCATE_MEMORY:
    fprintf(stderr, "Couldn't allocate sufficient memory.\n");
    break;
  case EOT_OTHER_STDLIB_ERROR:
    fprintf(stderr, "There was an unknown system error.\n");
    break;
  case EOT_COMPRESSION_NOT_YET_IMPLEMENTED:
    fprintf(stderr, "MTX Compression has not yet been implemented in this version of libeot. The font could therefore not be converted.\n");
    break;
  default:
    fprintf(stderr, "Unknown error: this is a bug in libeot; it does not *necessarily* indicate a corrupted font file.\n");
    break;
  }

}

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    usage(argv[0]);
    return 1;
  }
  struct stat st;
  if(stat(argv[1], &st) != 0)
  {
    fprintf(stderr, "The file %s could not be opened.\n", argv[1]);
    return 1;
  }
  int fildes = open(argv[1], O_RDONLY);
  if (fildes == -1)
  {
    fprintf(stderr, "The file %s could not be opened.\n", argv[1]);
    return 1;
  }
  //FIXME
  const char *outFileName = argv[2];
  FILE *outFile = fopen(outFileName, "wb");
  if (outFile == NULL)
  {
    fprintf(stderr, "The file %s could not be opened for writing.\n", outFileName);
    return 1;
  }

  uint8_t *font = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fildes, 0);
  if (font == MAP_FAILED)
  {
    err(1, NULL);
  }
  struct EOTMetadata out;
  enum EOTError result = EOTfillMetadata(font, st.st_size, &out);
  if (result != EOT_SUCCESS)
  {
    printError(result, argv[1]);
    return 1;
  }

  enum EOTError writeResult = writeFontFile(font + out.fontDataOffset, out.fontDataSize, out.flags & TTEMBED_TTCOMPRESSED, out.flags & TTEMBED_XORENCRYPTDATA, outFile);
  if (writeResult != EOT_SUCCESS)
  {
    printError(writeResult, outFileName);
    return 1;
  }
  fclose(outFile);
} 
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
