/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <libeot/libeot.h>

#include "MTXMEM.H"
#include "BITIO.H"
#include "AHUFF.H"
#include "LZCOMP.H"
#include "ERRCODES.H"
#include "../util/stream.h"

unsigned be24ToCpu(const uint8_t *buf)
{
	return ((unsigned)buf[2]) | (((unsigned)buf[1]) << 8) | (((unsigned)buf[0]) << 16);
}
enum EOTError unpackMtx(struct Stream *buf, unsigned size, uint8_t **bufsOut, unsigned *bufSizesOut)
{
	for (unsigned i = 0; i < 3; ++i)
	{
		bufsOut[i] = NULL;
	}
	enum StreamResult sResult;
	enum EOTError returnedStatus = EOT_SUCCESS;
	MTX_MemHandler *mem = MTX_mem_Create(&malloc, &realloc, &free);
	if (!mem)
	{
		goto CLEANUP;
	}
	LZCOMP *lzcomp = MTX_LZCOMP_Create1(mem);
	if (!lzcomp)
	{
		goto CLEANUP;
	}
	uint8_t versionMagic;
	uint32_t offsets[3];
	offsets[0] = 10;
	uint32_t copyLimit;
	sResult = BEReadU8(buf, &versionMagic);
	CHK_CN(sResult, EOT_MTX_ERROR);
	sResult = BEReadU24(buf, &copyLimit);
	CHK_CN(sResult, EOT_MTX_ERROR);
	for (unsigned i = 1 /* sic */; i < 3; ++i)
	{
		sResult = BEReadU24(buf, &offsets[i]);
		CHK_CN(sResult, EOT_MTX_ERROR);
	}
	unsigned sizes[] = {offsets[1] - offsets[0], offsets[2] - offsets[1], buf->size - offsets[2]};
	for (unsigned i = 0; i < 3; ++i)
	{
		if (offsets[i] + sizes[i] > buf->size)
		{
			returnedStatus = EOT_MTX_ERROR;
			goto CLEANUP;
		}
		long sizeOut;
		bufsOut[i] = (uint8_t *)MTX_LZCOMP_UnPackMemory(lzcomp, buf->buf + offsets[i], sizes[i], &sizeOut, versionMagic);
		bufSizesOut[i] = sizeOut;
		if (!bufsOut[i])
		{
			returnedStatus = EOT_MTX_ERROR;
			goto CLEANUP;
		}
	}
CLEANUP:
	MTX_LZCOMP_Destroy(lzcomp);
	free(mem);
	return returnedStatus;
}
#ifdef LZCOMP_MAIN
void usage(char *arg)
{
	fprintf(stderr, "Usage: %s font.mtx font.ctf\n", arg);
}
int main(int argc, char **argv)
{
	if (argc != 3)
	{
		usage(argv[0]);
		return 1;
	}
	MTX_MemHandler *mem = MTX_mem_Create(&malloc, &realloc, &free);
	LZCOMP *lzcomp = MTX_LZCOMP_Create1(mem);
	FILE *in = fopen(argv[1], "rb");
	if (in == NULL)
	{
		fprintf(stderr, "Cannot open file: %s\n", argv[1]);
		return 1;
	}
	uint8_t versionMagic;
	const unsigned block1Offset = 10;
	unsigned offsets[3];
	offsets[0] = 10;
	unsigned copyLimit;
	unsigned block2Offset, block3Offset;
	uint8_t buf24[3];
	fread(&versionMagic, 1, 1, in);
	fread(buf24, 1, 3, in);
	copyLimit = be24ToCpu(buf24);
	fread(buf24, 1, 3, in);
	offsets[1] = be24ToCpu(buf24);
	fread(buf24, 1, 3, in);
	offsets[2] = be24ToCpu(buf24);
	fseek(in, 0, SEEK_END);
	unsigned totalFileSize = ftell(in);
	unsigned sizes[] = {offsets[1] - offsets[0], offsets[2] - offsets[1], totalFileSize - offsets[2]};
	for (unsigned i = 0; i < 3; ++i)
	{
		char *buf = malloc(sizes[i]);
		fseek(in, offsets[i], SEEK_SET);
		fread(buf, 1, sizes[i], in);
		char fnBuf[10];
		sprintf(fnBuf, "%d.ctf", i + 1);
		FILE *out = fopen(fnBuf, "wb");
		if (out == NULL)
		{
			fprintf(stderr, "Cannot open file: %s for writing\n", fnBuf);
			return 1;
		}
		long sizeOut;
		char *outBuf = MTX_LZCOMP_UnPackMemory(lzcomp, buf, sizes[i], &sizeOut, versionMagic);
		fwrite(outBuf, 1, sizeOut, out);
		fclose(out);
		free(buf);
		free(outBuf);
	}
}
#endif
