/*
 *   rope.c -- May 8 2016 04:49:12 EDT
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

#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <check.h>

#include <foxbot/memory.h>
#include <foxbot/rope.h>

START_TEST(rope_check)
{
    rope r = ROPE_EMPTY;
    ck_assert_ptr_eq(shift_rope(&r), NULL);
    append_rope(&r, xalloc_rope_segment(":3", 3));
    append_rope(&r, xalloc_rope_segment("^.^", 4));
    ck_assert_uint_eq(rope_len(&r), 7);
    ck_assert_uint_eq(rope_count(&r), 2);
    {
        struct rope_segment *s = shift_rope(&r);
        ck_assert_str_eq(s->data, ":3");
        xfree(s);
    }
    {
        struct rope_segment *s = shift_rope(&r);
        ck_assert_str_eq(s->data, "^.^");
        xfree(s);
    }
}
END_TEST

static const size_t num_iter = 10000;

static void *
writer(void *p_r)
{
    tsrope *const r = p_r;
    for (unsigned i = 0; i < num_iter; ++i) {
        const unsigned data[4] = {i, ~i, i + 1, i - 1};
        append_tsrope(r, xalloc_rope_segment(data, sizeof(data)));
    }
    return NULL;
}

static void *
reader(void *p_r)
{
    tsrope *const r = p_r;
    for (unsigned i = 0; i < num_iter; ++i) {
        const unsigned expected_data[4] = {i, ~i, i + 1, i - 1};
        struct rope_segment *const s = waitshift_tsrope(r);
        ck_assert_uint_eq(s->len, sizeof(expected_data));
        unsigned data[sizeof(expected_data) / sizeof(*expected_data)];
        memcpy(data, s->data, sizeof(data));
        for (unsigned j = 0; j < sizeof(data) / sizeof(*data); ++j)
            ck_assert_uint_eq(expected_data[j], data[j]);
        xfree(s);
    }
    return NULL;
}

START_TEST(tsrope_check)
{
    tsrope r = TSROPE_EMPTY;
    pthread_t twriter, treader;
    ck_assert_ptr_eq(shift_tsrope(&r), NULL);
    append_tsrope(&r, xalloc_rope_segment(":3", 3));
    append_tsrope(&r, xalloc_rope_segment("^.^", 4));
    {
        struct rope_segment *s = shift_tsrope(&r);
        ck_assert_str_eq(s->data, ":3");
        xfree(s);
    }
    {
        struct rope_segment *s = shift_tsrope(&r);
        ck_assert_str_eq(s->data, "^.^");
        xfree(s);
    }
    ck_assert_int_eq(pthread_create(&twriter, NULL, writer, &r), 0);
    ck_assert_int_eq(pthread_create(&treader, NULL, reader, &r), 0);
    ck_assert_int_eq(pthread_join(twriter, NULL), 0);
    ck_assert_int_eq(pthread_join(treader, NULL), 0);
}
END_TEST

void
rope_setup(Suite *s)
{
    TCase *tc = tcase_create("rope");
    tcase_add_test(tc, rope_check);
    tcase_add_test(tc, tsrope_check);
    suite_add_tcase(s, tc);
}
