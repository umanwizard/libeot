/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_TRIPLET_ENCODINGS_H__
#define __LIBEOT_TRIPLET_ENCODINGS_H__

struct TripletEncoding
{
  unsigned byteCount;
  unsigned xBits;
  unsigned yBits;
  unsigned deltaX;
  unsigned deltaY;
  int xSign;
  int ySign;
};

extern struct TripletEncoding tripletEncodings[];

#endif /* #define __LIBEOT_TRIPLET_ENCODINGS_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
