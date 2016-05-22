/*
 *   error.h -- May 8 2016 03:49:44 EDT
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

#ifndef FOX_ERROR_H_
#define FOX_ERROR_H_

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#define panic(...) panic_(__FILE__, __LINE__, __func__, "" __VA_ARGS__);

#define xensure0(expr)                                  \
    xensure0_((expr), __FILE__, __LINE__, __func__,     \
              "returned %i instead of zero: " #expr);   \

void panic_message(const char *file,
                   uintmax_t line,
                   const char *func,
                   const char *format,
                   ...);

void vpanic_message(const char *file,
                    uintmax_t line,
                    const char *func,
                    const char *format,
                    va_list vlist);

static inline void
panic_(const char *file,
       uintmax_t line,
       const char *func,
       const char *format,
       ...)
{
    va_list vlist;
    va_start(vlist, format);
    vpanic_message(file, line, func, format, vlist);
    va_end(vlist);
    abort();
}

static inline int
xensure0_(int value,
          const char *file,
          uintmax_t line,
          const char *func,
          const char *format)
{
    if (value) {
        panic_message(file, line, func, format, value);
        abort();
    }
    return value;
}

#ifdef COVERAGE
#undef panic
#define panic(...) (void)0
#endif

void fox_strerror(int errnum, char *buf, size_t buflen);

#endif
