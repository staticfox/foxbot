/*
 *   error.c -- May 8 2016 03:56:56 EDT
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

#define _POSIX_C_SOURCE 200112L

#include <config.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "error.h"

void
panic_message(const char *file,
              uintmax_t line,
              const char *func,
              const char *format,
              ...)
{
    va_list vlist;
    va_start(vlist, format);
    vpanic_message(file, line, func, format, vlist);
    va_end(vlist);
}

void
vpanic_message(const char *file,
              uintmax_t line,
              const char *func,
              const char *format,
               va_list vlist)
{
    char msg[1024];
    vsnprintf(msg, sizeof(msg), format, vlist);
    fprintf(stderr, PACKAGE_NAME ":%s:%lu:%s: %s\n", file, line, func, msg);
    fflush(stderr);
}

void
fox_strerror(int errnum, char *buf, size_t buflen)
{
    static const char default_msg[] = "Unknown error";
#ifdef STRERROR_R_CHAR_P
    char *msg;
    if ((msg = strerror_r(errnum, buf, buflen)) {
        if (!(msg >= buf && msg < buf + buflen)) {
            strncpy(buf, msg, buflen);
            buf[buflen - 1] = '\0';
        }
        return;
    }
#else
    if (!strerror_r(errnum, buf, buflen))
        return;
#endif
    if (buflen < sizeof(default_msg)) {
        fprintf(stderr, PACKAGE_NAME ":fox_strerror: buffer too small\n");
        fflush(stderr);
        abort();
    }
    strcpy(buf, default_msg);
}
