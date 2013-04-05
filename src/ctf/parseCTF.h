/* Copyright (c) 2012 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is license under the MIT license.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_PARSE_CTF_H__
#define __LIBEOT_PARSE_CTF_H__

#include "../EOTError.h"
#include "../util/stream.h"
#include "SFNTContainer.h"

enum EOTError parseCTF(struct Stream **streams, struct SFNTContainer **out);

#endif /* #define __LIBEOT_PARSE_CTF_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
