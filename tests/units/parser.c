/*
 *   parser.c -- May 8 2016 04:49:12 EDT
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

#include <limits.h>
#include <stdio.h>
#include <check.h>

#include <foxbot/memory.h>
#include <foxbot/parser.h>

START_TEST(parse_int_check)
{
    {
        char s[] = "12";
        char *p = s;
        int n = -42;
        ck_assert_int_eq(parse_int(&p, &n), 1);
        ck_assert_int_eq(n, 12);
        ck_assert_ptr_eq(p, s + strlen(s));
    }
    {
        char s[] = "";
        char *p = s;
        int n = -42;
        ck_assert_int_eq(parse_int(&p, &n), 0);
        ck_assert_int_eq(n, -42);
        ck_assert_ptr_eq(p, s);
    }
    {
        char s[] = " 12";
        char *p = s;
        int n = -42;
        ck_assert_int_eq(parse_int(&p, &n), 0);
        ck_assert_int_eq(n, -42);
        ck_assert_ptr_eq(p, s);
    }
    {
        /* test a number that's out of range */
        char *s = xmalloc(snprintf(NULL, 0, "%i", INT_MAX) + 1);
        s[0] = '1';
        sprintf(s + 1, "%i", INT_MAX);
        char *p = s;
        int n = -42;
        ck_assert_int_eq(parse_int(&p, &n), 0);
        ck_assert_int_eq(n, -42);
        ck_assert_ptr_eq(p, s);
        xfree(s);
    }
}
END_TEST

START_TEST(parse_long_check) {
    {
        char s[] = "12";
        char *p = s;
        long n = -42;
        ck_assert_int_eq(parse_long(&p, &n), 1);
        ck_assert_int_eq(n, 12);
        ck_assert_ptr_eq(p, s + strlen(s));
    }
    {
        char s[] = "";
        char *p = s;
        long n = -42;
        ck_assert_int_eq(parse_long(&p, &n), 0);
        ck_assert_int_eq(n, -42);
        ck_assert_ptr_eq(p, s);
    }
    {
        char s[] = " 12";
        char *p = s;
        long n = -42;
        ck_assert_int_eq(parse_long(&p, &n), 0);
        ck_assert_int_eq(n, -42);
        ck_assert_ptr_eq(p, s);
    }
    {
        /* test a number that's out of range */
        char *s = xmalloc(snprintf(NULL, 0, "%li", LONG_MAX) + 1);
        s[0] = '1';
        sprintf(s + 1, "%li", LONG_MAX);
        char *p = s;
        long n = -42;
        ck_assert_int_eq(parse_long(&p, &n), 0);
        ck_assert_int_eq(n, -42);
        ck_assert_ptr_eq(p, s);
        xfree(s);
    }
}
END_TEST

START_TEST(parse_uint_check) {
    {
        char s[] = "12";
        char *p = s;
        unsigned n = 42;
        ck_assert_int_eq(parse_uint(&p, &n), 1);
        ck_assert_int_eq(n, 12);
        ck_assert_ptr_eq(p, s + strlen(s));
    }
    {
        char s[] = "";
        char *p = s;
        unsigned n = 42;
        ck_assert_int_eq(parse_uint(&p, &n), 0);
        ck_assert_int_eq(n, 42);
        ck_assert_ptr_eq(p, s);
    }
    {
        char s[] = " 12";
        char *p = s;
        unsigned n = 42;
        ck_assert_int_eq(parse_uint(&p, &n), 0);
        ck_assert_int_eq(n, 42);
        ck_assert_ptr_eq(p, s);
    }
    {
        /* test a number that's out of range */
        char *s = xmalloc(snprintf(NULL, 0, "%u", UINT_MAX) + 1);
        s[0] = '1';
        sprintf(s + 1, "%u", UINT_MAX);
        char *p = s;
        unsigned n = 42;
        ck_assert_int_eq(parse_uint(&p, &n), 0);
        ck_assert_int_eq(n, 42);
        ck_assert_ptr_eq(p, s);
        xfree(s);
    }
}
END_TEST

START_TEST(parse_ulong_check) {
    {
        char s[] = "12";
        char *p = s;
        unsigned long n = 42;
        ck_assert_int_eq(parse_ulong(&p, &n), 1);
        ck_assert_int_eq(n, 12);
        ck_assert_ptr_eq(p, s + strlen(s));
    }
    {
        char s[] = "";
        char *p = s;
        unsigned long n = 42;
        ck_assert_int_eq(parse_ulong(&p, &n), 0);
        ck_assert_int_eq(n, 42);
        ck_assert_ptr_eq(p, s);
    }
    {
        char s[] = " 12";
        char *p = s;
        unsigned long n = 42;
        ck_assert_int_eq(parse_ulong(&p, &n), 0);
        ck_assert_int_eq(n, 42);
        ck_assert_ptr_eq(p, s);
    }
    {
        /* test a number that's out of range */
        char *s = xmalloc(snprintf(NULL, 0, "%lu", ULONG_MAX) + 1);
        s[0] = '1';
        sprintf(s + 1, "%lu", ULONG_MAX);
        char *p = s;
        unsigned long n = 42;
        ck_assert_int_eq(parse_ulong(&p, &n), 0);
        ck_assert_int_eq(n, 42);
        ck_assert_ptr_eq(p, s);
        xfree(s);
    }
}
END_TEST

void
parser_setup(Suite *s)
{
    TCase *tc = tcase_create("parser");
    tcase_add_test(tc, parse_int_check);
    tcase_add_test(tc, parse_long_check);
    tcase_add_test(tc, parse_uint_check);
    tcase_add_test(tc, parse_ulong_check);
    suite_add_tcase(s, tc);
}
