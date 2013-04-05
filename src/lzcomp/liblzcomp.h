#ifndef __LIBEOT_LIBLZCOMP_H__
#define __LIBEOT_LIBLZCOMP_H__

#include "../EOTError.h"
#include "../util/stream.h"

enum EOTError unpackMtx(struct Stream *buf, unsigned size, uint8_t **bufsOut, unsigned *bufSizesOut);

#endif
