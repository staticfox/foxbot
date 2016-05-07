/*
 *   foxsocket.h -- April 28 2016 05:19:58 EDT
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

#ifndef FOX_SOCKET_H_
#define FOX_SOCKET_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char *_buf, *_line_start;
    size_t _buf_size, _buf_used;
    int _fd;
} io_state;

enum io_readline_status {
    IRS_OK,
    IRS_CLOSED,
    IRS_NOBUFS,
    IRS_ERROR
};

void create_socket(void);
void destroy_socket(void);
void establish_link(void);
void sockwrite(const char *buf);
void raw(char *fmt, ...);
void init_io(io_state *io, int fd, size_t buf_size);
void reset_io(io_state *io);
int get_io_fd(const io_state *io);

/** The returned buffer can be modified but must not be freed. */
enum io_readline_status io_readline(io_state *io, char **line);

/** Simpler version of `io_readline` that prints a message with the given
    `prefix` and returns `NULL` if an error occurs. */
char *io_simple_readline(io_state *io, const char *prefix);

#endif
