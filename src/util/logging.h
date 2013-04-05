/* Copyright (c) 2012 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is license under the MIT license.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_LOGGING_H__
#define __LIBEOT_LOGGING_H__

#include <stdio.h>

void logWarning(const char *out)
{
  fputs(out, stderr);
}

#endif /* #define __LIBEOT_LOGGING_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
