/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_LIBLZCOMP_H__
#define __LIBEOT_LIBLZCOMP_H__

#include <libeot/libeot.h>

#include "../util/stream.h"

enum EOTError unpackMtx(struct Stream *buf, unsigned size, uint8_t **bufsOut, unsigned *bufSizesOut);

#endif
