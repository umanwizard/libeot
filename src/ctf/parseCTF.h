/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_PARSE_CTF_H__
#define __LIBEOT_PARSE_CTF_H__

#include <libeot/libeot.h>

#include "../util/stream.h"
#include "SFNTContainer.h"

enum EOTError parseCTF(struct Stream **streams, struct SFNTContainer **out);

#endif /* #define __LIBEOT_PARSE_CTF_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
