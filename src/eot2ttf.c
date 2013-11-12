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

#include <libeot/libeot.h>

#include "flags.h"
#include "writeFontFile.h"

void usage(char *progName)
{
  fprintf(stderr, "Usage: %s myfont.eot out.ttf\n", progName);
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

  const uint8_t *font = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fildes, 0);
  if (font == MAP_FAILED)
  {
    err(1, NULL);
  }
  struct EOTMetadata out;
  enum EOTError result = EOT2ttf_file(font, st.st_size, &out, outFile);
  if (result != EOT_SUCCESS)
  {
    EOTprintError(result, stderr);
    return 1;
  }
  EOTfreeMetadata(&out);
  fclose(outFile);
} 
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
