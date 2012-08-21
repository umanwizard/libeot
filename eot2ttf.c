/* Copyright (c) 2012 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is license under the MIT license.
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

void usage(char *progName)
{
  fprintf(stderr, "Usage: %s myfont.eot\n", progName);
}

int main(int argc, char **argv)
{
  if (argc != 2)
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

  uint8_t *font = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fildes, 0);
  if (font == MAP_FAILED)
  {
    err(1, NULL);
  }
  struct EOTMetadata out;
  enum EOTError result = EOTfillMetadata(font, st.st_size, &out);
  switch (result)
  {
  case EOT_SUCCESS:
    printf("Getting metadata apparently succeeded.\n");
    return 0;
  case EOT_INSUFFICIENT_BYTES:
    fprintf(stderr, "The file %s appears truncated.\n", argv[1]);
    return 1;
  case EOT_BOGUS_STRING_SIZE:
  case EOT_CORRUPT_FILE:
    fprintf(stderr, "The file %s appears corrupt.\n", argv[1]);
    return 1;
  case EOT_CANT_ALLOCATE_MEMORY:
    fprintf(stderr, "Couldn't allocate sufficient memory.\n");
    return 1;
  case EOT_OTHER_STDLIB_ERROR:
    fprintf(stderr, "There was an unknown system error.\n");
    return 1;
  default:
    fprintf(stderr, "Unknown error: this is a bug.\n");
    return 1;
  }
} 
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
