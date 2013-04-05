/* Copyright (c) 2012 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is license under the MIT license.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_WRITE_FONT_FILE_H__
#define __LIBEOT_WRITE_FONT_FILE_H__

#include <stdbool.h>
#include <stdint.h>

#include "EOTError.h"

enum EOTError writeFontFile(uint8_t *font, unsigned fontSize, bool compressed, bool encrypted, FILE *outFile);

#endif /* #define __LIBEOT_WRITE_FONT_FILE_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
