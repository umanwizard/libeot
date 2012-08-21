/* Copyright (c) 2012 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is license under the MIT license.
 * For full details, see the file LICENSE
 */

#ifndef __LIBEOT_EOTERROR_H__
#define __LIBEOT_EOTERROR_H__

enum EOTError
{
  EOT_SUCCESS,
  EOT_INSUFFICIENT_BYTES,
  EOT_BOGUS_STRING_SIZE,
  EOT_CORRUPT_FILE,
  EOT_CANT_ALLOCATE_MEMORY,
  EOT_OTHER_STDLIB_ERROR
};
#endif /* #define __LIBEOT_EOTERROR_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
