/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#include <stdint.h>

unsigned umax(unsigned a, unsigned b)
{
  return a > b ? a : b;
}

int imax(int a, int b)
{
  return a > b ? a : b;
}

int imin(int a, int b)
{
  return a < b ? a : b;
}

int16_t i16max(int16_t a, int16_t b)
{
  return (int16_t)imax((int)a, (int)b);
}

int16_t i16min(int16_t a, int16_t b)
{
  return (int16_t)imin((int)a, (int)b);
}



/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
