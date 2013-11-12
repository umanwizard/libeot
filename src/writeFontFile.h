/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_WRITE_FONT_FILE_H__
#define __LIBEOT_WRITE_FONT_FILE_H__

#include <stdbool.h>
#include <stdint.h>

#include <libeot/libeot.h>

enum EOTError writeFontBuffer(const uint8_t *font, unsigned fontSize, bool compressed, bool encrypted, uint8_t **finalOutBuffer, unsigned *finalFontSize);

enum EOTError writeFontFile(const uint8_t *font, unsigned fontSize, bool compressed, bool encrypted, FILE *outFile);

#endif /* #define __LIBEOT_WRITE_FONT_FILE_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
