/*
 *   parser.c -- May 8 2016 21:56:23 EDT
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

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

#include <foxbot/parser.h>

bool
strip_prefix(const char *str, const char *prefix, char **suffix)
{
    for (; *prefix; ++str, ++prefix)
        if (*str != *prefix)
            return false;
    if (suffix)
        *suffix = (char *)str;
    return true;
}

bool
iparse_int(const char **string, int *value_out)
{
    long value;
    const char *s = *string;

    if (!iparse_long(&s, &value) || value > INT_MAX || value < INT_MIN)
        return 0;

    *value_out = (int)value;
    *string = s;
    return 1;
}

bool
parse_uint(const char *str, unsigned *value_out)
{
    unsigned value;
    return iparse_uint(&str, &value) && !*str && (*value_out = value);
}

bool
iparse_uint(const char **string, unsigned *value_out)
{
    unsigned long value;
    const char *s = *string;

    if (!iparse_ulong(&s, &value) || value > UINT_MAX)
        return 0;

    *value_out = (unsigned)value;
    *string = s;
    return 1;
}

bool
parse_long(const char *str, long *value_out)
{
    long value;
    return iparse_long(&str, &value) && !*str && (*value_out = value);
}

bool
iparse_long(const char **string, long *value_out)
{
    const char *const nptr = *string;
    char *endptr;
    long value;

    /* forbid initial whitespace */
    if (isspace(nptr[0]))
        return 0;

    errno = 0;
    value = strtol(nptr, &endptr, 10);
    if (errno || endptr == nptr)
        return 0;

    *value_out = value;
    *string = endptr;
    return 1;
}

bool
iparse_ulong(const char **string, unsigned long *value_out)
{
    const char *const nptr = *string;
    char *endptr;
    unsigned long value;

    /* forbid initial whitespace */
    if (isspace(nptr[0]))
        return 0;

    errno = 0;
    value = strtoul(nptr, &endptr, 10);
    if (errno || endptr == nptr)
        return 0;

    *value_out = value;
    *string = endptr;
    return 1;
}
