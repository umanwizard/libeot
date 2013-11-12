/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_LIBEOT_H__
#define __LIBEOT_LIBEOT_H__

#include <stdint.h>
#include <stdio.h>

#include "EOT.h"
#include "EOTError.h"

enum EOTError EOT2ttf_file(const uint8_t *font, unsigned fontSize, struct EOTMetadata *metadataOut, FILE *out);
enum EOTError EOT2ttf_buffer(const uint8_t *font, unsigned fontSize, struct EOTMetadata *metadataOut, uint8_t **fontOut,
    unsigned *fontSizeOut);

void EOTfreeBuffer(const uint8_t *buffer);
void EOTprintError(enum EOTError, FILE *out);

#endif /* #define __LIBEOT_LIBEOT_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
