/* Copyright (c) 2013 Brennan T. Vincent <brennanv@email.arizona.edu>
 * This file is a part of libeot, which is licensed under the MPL license, version 2.0.
 * For full details, see the file LICENSE
 */
#include <string.h> /* for size_t */
#include <stdlib.h>
#include "MTXMEM.H"

void *MTX_mem_malloc(MTX_MemHandler *t, unsigned long size)
{
    return t->malloc(size);
}

void *MTX_mem_realloc(MTX_MemHandler *t, void *p, unsigned long size)
{
    return t->realloc(p, size);
}

void  MTX_mem_free(MTX_MemHandler *t, void *deadObject)
{
    t->free(deadObject);
}


MTX_MemHandler *MTX_mem_Create(MTX_MALLOCPTR mptr, MTX_REALLOCPTR rptr, MTX_FREEPTR fptr)
{
    MTX_MemHandler *t = (MTX_MemHandler *)malloc(sizeof(MTX_MemHandler));
    *t = (MTX_MemHandler){0};
    t->malloc = mptr;
    t->realloc = rptr;
    t->free = fptr;
    return t;
}

