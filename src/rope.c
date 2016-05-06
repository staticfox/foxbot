/*
 *   rope.c -- May 6 2016 05:02:42 EDT
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

#include <config.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <foxbot/memory.h>
#include <foxbot/rope.h>

struct rope_segment *
alloc_rope_segment(const char *data, size_t len)
{
    static const struct rope_segment ROPE_SEGMENT_EMPTY;
    struct rope_segment *const s = xmalloc(sizeof(*s) + len);
    *s = ROPE_SEGMENT_EMPTY;
    s->len = len;
    memcpy(s->data, data, len);
    return s;
}

void
append_rope(rope *r, struct rope_segment *s)
{
    s->next = NULL;
    if (r->_tail) {
        assert(!r->_tail->next);
        r->_tail->next = s;
    } else {
        r->_head = s;
    }
    r->_tail = s;
}

struct rope_segment *
shift_rope(rope *r)
{
    struct rope_segment *const s = r->_head;
    if (s) {
        r->_head = s->next;
        if (!s->next) {
            assert(r->_tail == s);
            r->_tail = NULL;
        }
        s->next = NULL;
    }
    return s;
}

void
clear_rope(rope *r)
{
    struct rope_segment *s;
    while ((s = shift_rope(r))) {
        xfree(s);
    }
}

void
append_tsrope(tsrope *r, struct rope_segment *s)
{
    pthread_mutex_lock(&r->_mutex);
    append_rope(&r->_rope, s);
    /* I think pthread_cond_signal should be safe here, but I'm using
       broadcast just to be sure */
    pthread_cond_broadcast(&r->_cond);
    pthread_mutex_unlock(&r->_mutex);
}

struct rope_segment *
shift_tsrope(tsrope *r)
{
    pthread_mutex_lock(&r->_mutex);
    struct rope_segment *const s = shift_rope(&r->_rope);
    pthread_mutex_unlock(&r->_mutex);
    return s;
}

struct rope_segment *
waitshift_tsrope(tsrope *r)
{
    pthread_mutex_lock(&r->_mutex);
    struct rope_segment *s;
    while (!(s = shift_rope(&r->_rope))) {
        pthread_cond_wait(&r->_cond, &r->_mutex);
    }
    pthread_mutex_unlock(&r->_mutex);
    return s;
}

void
clear_tsrope(tsrope *r)
{
    pthread_mutex_lock(&r->_mutex);
    clear_rope(&r->_rope);
    pthread_mutex_unlock(&r->_mutex);
}
