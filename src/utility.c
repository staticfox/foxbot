/*
 *   utility.c -- Aug 9 2016 01:28:16 EDT
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

#include <ctype.h>
#include <string.h>

void
fox_toupper(char *str)
{
    for (size_t ii = 0; str[ii]; ++ii)
        str[ii] = toupper(str[ii]);
}

char *
fox_strsep(char **stringp, const char *delim)
{
    char *p = *stringp;
    if (p) {
        char *q = p + strcspn(p, delim);
        *stringp = *q ? q + 1 : NULL;
        *q = '\0';
    }
    return p;
}
