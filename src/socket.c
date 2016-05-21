/*
 *   foxsocket.c -- April 28 2016 05:18:14 EDT
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

#define _POSIX_C_SOURCE 201112L

#include <config.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <foxbot/conf.h>
#include <foxbot/foxbot.h>
#include <foxbot/message.h>
#include <foxbot/socket.h>

void
create_socket(void)
{
    int addrerr;
    static const struct addrinfo ADDRINFO_EMPTY;
    struct addrinfo hints = ADDRINFO_EMPTY;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((addrerr = getaddrinfo(botconfig.host, botconfig.port, &hints, &bot.hil)) != 0) {
        do_error("getaddrinfo() returned %d. %s",
                 addrerr, addrerr == EAI_SYSTEM ? strerror(errno) : "");
        exit(EXIT_FAILURE);
    }

    int fd = socket(bot.hil->ai_family, bot.hil->ai_socktype, bot.hil->ai_protocol);
    if (fd == -1) {
        do_error(strerror(errno));
        exit(EXIT_FAILURE);
    }
    init_io(&bot.io, fd, MAX_IRC_BUF);
}

void
destroy_socket(void)
{
    const int fd = get_io_fd(&bot.io);
    reset_io(&bot.io);

    /* Terminate connection cleanly */
    shutdown(fd, SHUT_WR);
    char buf[BUFSIZ];
    while (recv(fd, buf, sizeof(buf), 0) > 0);
    close(fd);

    freeaddrinfo(bot.hil);
    bot.hil = NULL;
}

void
establish_link(void)
{
    int status;

    status = connect(get_io_fd(&bot.io), bot.hil->ai_addr, bot.hil->ai_addrlen);
    if (status == -1) {
        do_error(strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (botconfig.password)
        raw("PASS :%s\n", botconfig.password);

    raw("CAP LS\n");
    raw("NICK %s\n", botconfig.nick);
    raw("USER %s 0 0 :%s\n", botconfig.ident, botconfig.realname);
}

void
sockwrite(const char *buf)
{
    ssize_t writeval = strlen(buf);
    ssize_t retval = write(get_io_fd(&bot.io), buf, writeval);

    if (retval == -1) {
        fprintf(stderr, "Write error: %s\n", strerror(errno));
        exit(EXIT_FAILURE); /* rip */
    }

    /* Warn? */
    if (writeval != retval) {
        fprintf(stderr, "Write error: Data content count mis-match\n");
        return;
    }
}

void
raw(char *fmt, ...)
{
    char sbuf[MAX_IRC_BUF] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(sbuf, MAX_IRC_BUF, fmt, ap);
    va_end(ap);

    printf("<< %s", sbuf);
    sockwrite(sbuf);
}

void
init_io(io_state *io, int fd, size_t buf_size)
{
    io->_buf = (char *)xmalloc(buf_size);
    io->_line_start = io->_buf;
    io->_buf_size = buf_size;
    io->_buf_used = 0;
    io->_fd = fd;
}

void
reset_io(io_state *io)
{
    io->_fd = -1;
    io->_buf_used = 0;
    io->_buf_size = 0;
    io->_line_start = NULL;
    xfree(io->_buf);
    io->_buf = NULL;
}

int
get_io_fd(const io_state *io)
{
    return io->_fd;
}

enum io_readline_status
io_readline(io_state *io, char **line)
{
    /* Scan for newlines in the line buffer; we're careful here to deal with embedded \0s
     * a broken server may send, as well as only processing lines that are complete. */
    char *line_end;
    while (!(line_end = memchr(io->_line_start, '\n', io->_buf_used))) {

        /* Shift buffer down so the unprocessed data is at the start */
        memmove(io->_buf, io->_line_start, io->_buf_used);

        /* See if there is any buffer remaining */
        const size_t buf_remain = io->_buf_size - io->_buf_used;
        if (buf_remain == 0) {
            return IRS_NOBUFS;
        }

        /* Receive some data */
        const ssize_t rv = recv(io->_fd, io->_buf + io->_buf_used, buf_remain, 0);
        if (rv == 0) {
            return IRS_CLOSED;
        } else if (rv < 0) {
            return IRS_ERROR;
        }

        io->_buf_used += rv;
        io->_line_start = io->_buf;
    }

    /* Delete the newline character(s) */
    *line_end = '\0';
    if (io->_line_start != line_end && line_end[-1] == '\r')
        line_end[-1] = '\0';

    /* Return the line */
    *line = io->_line_start;

    /* Update the state to point to the next line */
    io->_buf_used -= line_end + 1 - io->_line_start;
    io->_line_start = line_end + 1;

    return IRS_OK;
}

char *
io_simple_readline(io_state *io, const char *prefix)
{
    char *line;
    switch (io_readline(io, &line)) {
    case IRS_OK:
        return line;
    case IRS_CLOSED:
        fprintf(stderr, "%sRemote host closed the connection\n", prefix);
        return NULL;
    case IRS_NOBUFS:
        fprintf(stderr, "%sLine exceeded buffer length\n", prefix);
        return NULL;
    case IRS_ERROR:
        if (errno != EINTR)
            fprintf(stderr, "%sRead error: %s\n", prefix, strerror(errno));
        return NULL;
    }
    abort();
}
