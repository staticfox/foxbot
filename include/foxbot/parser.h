/*
 *   parser.h -- May 8 2016 21:52:49 EDT
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

#ifndef FOX_PARSER_H_
#define FOX_PARSER_H_

#include <stdbool.h>

/** Parse a decimal integer.  If the integer could not be parsed (because it
 * was out of range or not a valid integer, `false` is returned and the
 * arguments are not modified.  Otherwise, `true` is returned, the value is
 * saved in `value_out`, and `string` is moved past the parsed integer. */
bool parse_int(char **string, int *value_out);

/** Works just like `parse_int`. */
bool parse_uint(char **string, unsigned *value_out);

/** Works just like `parse_int`. */
bool parse_long(char **string, long *value_out);

/** Works just like `parse_int`. */
bool parse_ulong(char **string, unsigned long *value_out);

#endif
