/*
 *   memory.c -- April 27 2016 11:28:55 EST
 *
 *   This file is part of the foxbot IRC bot
 *   Copyright (C) 2016 Matt Ullman (staticfox at staticfox dot net)
 *
 *   This program is FREE software. You can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 *
 *   This program is distributed in the HOPE that it will be USEFUL,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "memory.h"
#include "stdinc.h"

#ifdef WANT_MEMDEBUG
# define MEMDEBUG(f, ...) printf((f), __VA_ARGS__)
#else
# define MEMDEBUG(f, ...)
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifndef HAVE_MALLOC_USABLE_SIZE
#define malloc_usable_size(x) 0
#endif

static int alive = 1;
unsigned long long heap = { 0 };
unsigned long long heap_max = { 0 };
unsigned long long num_allocs = { 0 };

static unsigned long long
max(unsigned long long x, unsigned long long y)
{
    return x > y ? x : y;
}

void *
xmalloc(size_t bytes)
{
    void *ret = malloc(bytes);

    if (ret == NULL)
        outofmemory();

    if (alive) {
        MEMDEBUG("MEMDEBUG: malloc: %lu | heap: %llu\r\n", bytes, heap);
        heap += malloc_usable_size(ret);
        heap_max = max(heap_max, heap);
        ++num_allocs;
    }

    return ret;
}

void *
xcalloc(size_t n, size_t s)
{
    void *ret = calloc(n, s);

    if (ret == NULL)
        outofmemory();

    if (alive) {
        MEMDEBUG("MEMDEBUG: calloc: %lu | heap: %llu\r\n", (n * sizeof(s)), heap);
        heap += malloc_usable_size(ret);
        heap_max = max(heap_max, heap);
        ++num_allocs;
    }

    return ret;
}

void *
xrealloc(void *x, size_t y)
{
    if (alive) {
        MEMDEBUG("MEMDEBUG: realloc: %lu | heap: %llu\r\n", y, heap);
        heap -= malloc_usable_size(x);
    }

    void *ret = realloc(x, y);

    if (y && ret == NULL)
        outofmemory();

    if (alive) {
        heap += malloc_usable_size(ret);
        heap_max = max(heap_max, heap);
    }

    return ret;
}

void
xfree(void *x)
{
    if (alive) {
        MEMDEBUG("MEMDEBUG: free: %lu | heap: %llu\r\n", sizeof(*x), heap);
        heap -= malloc_usable_size(x);
    }

    free(x);
}

void *
xstrdup(const char *s)
{
    size_t size = strlen(s) + 1;
    void *ret = xmalloc(size);

    if (ret == NULL)
        outofmemory();

    memcpy(ret, s, size);

    return ret;
}

void
outofmemory(void)
{
    alive = 0;
    fprintf(stderr, "Out of memory! x.x\n");
    fflush(stderr);
    abort();
}
