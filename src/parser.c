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
parse_int(char **string, int *value_out)
{
    long value;
    char *s = *string;

    if (!parse_long(&s, &value) || value > INT_MAX || value < INT_MIN)
        return 0;

    *value_out = (int)value;
    *string = s;
    return 1;
}

bool
parse_uint(char **string, unsigned *value_out)
{
    unsigned long value;
    char *s = *string;

    if (!parse_ulong(&s, &value) || value > UINT_MAX)
        return 0;

    *value_out = (unsigned)value;
    *string = s;
    return 1;
}

bool
parse_long(char **string, long *value_out)
{
    char *const nptr = *string, *endptr;
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
parse_ulong(char **string, unsigned long *value_out)
{
    char *const nptr = *string, *endptr;
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
