/*
 *   rope.h -- May 6 2016 05:03:02 EDT
 *
 *   This file is part of the foxbot IRC bot
 *   Copyright (C) 2016 Fylwind Ruzkelt (fyl@wolfpa.ws)
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

#ifndef FOX_ROPE_H_
#define FOX_ROPE_H_

#include <stddef.h>
#include <pthread.h>

/** A rope segment.  The size of any rope_segment `r` is always
    `sizeof(r) + r.len` */
struct rope_segment {
    struct rope_segment *next;
    size_t len;
    char data[];
};

/** A singly linked list of char arrays. */
typedef struct {
    size_t _len;                        /* Total number of bytes */
    size_t _count;                      /* Number of segments */
    struct rope_segment *_head;
    struct rope_segment *_tail;
} rope;

/** An empty rope initializer. */
#define ROPE_EMPTY {0, 0, NULL, NULL}

/** Allocate a new rope segment with the given bytes as data.  The segment can
  * be later deallocated using `free`. */
struct rope_segment *xalloc_rope_segment(const void *data, size_t len);

/** Get the total number of bytes in a rope. */
static inline size_t rope_len(const rope *r) { return r->_len; }

/** Get the number of segments in a rope. */
static inline size_t rope_count(const rope *r) { return r->_count; }

/** Attach a new segment to the end of a rope. */
void append_rope(rope *r, struct rope_segment *s);

/** Remove the head and return it.  Be sure to free it later.  Returns `NULL`
    if the rope is empty. */
struct rope_segment *shift_rope(rope *r);

/** Remove all elements from the rope and free all its used memory. */
void clear_rope(rope *r);

/** Like `#rope`, but thread-safe. */
typedef struct {
    pthread_cond_t _cond;    /* Used to notify threads waiting for segments */
    pthread_mutex_t _mutex;
    rope _rope;
#ifndef NDEBUG
    char _is_initialized;
#endif
} tsrope;

#ifdef NDEBUG
#define TSROPE_EMPTY                                                    \
    {PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, ROPE_EMPTY}
#else
#define TSROPE_EMPTY                                                    \
    {PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, ROPE_EMPTY, 1}
#endif

/** Attach a new segment to the end of the rope. */
void append_tsrope(tsrope *r, struct rope_segment *s);

/** Remove the head and return it.  Be sure to free it later.  Returns `NULL`
    if the rope is empty. */
struct rope_segment *shift_tsrope(tsrope *r);

/** Like `#shift_tsrope` but will block indefinitely until a segment becomes
    available.  Hence, the return value is never `NULL`. */
struct rope_segment *waitshift_tsrope(tsrope *r);

/** Remove all elements from the rope and free all its used memory. */
void clear_tsrope(tsrope *r);

#endif
