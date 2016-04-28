/*
 *   foxsocket.c -- April 28 2016 05:18:14 EST
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

#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "conf.h"
#include "foxbot.h"
#include "socket.h"
#include "stdinc.h"

int
create_and_bind(void)
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /* TODO: Un-hardcode this. */
    if (getaddrinfo(botconfig.host, botconfig.port, &hints, &bot.hil) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    bot.fd = socket(bot.hil->ai_family, bot.hil->ai_socktype, bot.hil->ai_protocol);
    if (bot.fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int
establish_link(void)
{
    int status, flags;

    status = connect(bot.fd, bot.hil->ai_addr, bot.hil->ai_addrlen);

    if (status == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    if(-1 == (flags = fcntl(bot.fd, F_GETFL, 0)))
        flags = 0;

    fcntl(bot.fd, F_SETFL, flags | O_NONBLOCK);

    raw("NICK %s\n", botconfig.nick);
    raw("USER %s 0 0 :%s\n", botconfig.ident, botconfig.realname);

    return 0;
}

void
sockwrite(const char *buf)
{
    ssize_t writeval = strlen(buf);
    ssize_t retval = write(bot.fd, buf, writeval);

    if (retval == -1) {
        fprintf(stderr, "Write error: %s\n", strerror(errno));
        quitting = 1;
        return;
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
    char sbuf[MAX_IRC_BUF] = { 0 };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(sbuf, MAX_IRC_BUF, fmt, ap);
    va_end(ap);

    printf("<< %s", sbuf);
    sockwrite(sbuf);
}

/* Main IO loop */
void
io(void)
{
    char inbuf[MAX_IO_BUF], innbuf[MAX_IO_BUF];
    size_t inbuf_used = 0, buf_used = 0;
    size_t buf_remain = sizeof(inbuf) - inbuf_used;

    if (buf_remain == 0) {
        fprintf(stderr, "Line exceeded buffer length\n");
        quitting = 1;
        return;
    }

    ssize_t rv = recv(bot.fd, (void *)&inbuf[inbuf_used], buf_remain, 0);

    if (rv == 0) {
        fprintf(stderr, "Remote host closed the connection\n");
        quitting = 1;
        return;
    }

    /* no data for now, call back when the socket is readable */
    if (rv < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return;

    if (rv < 0) {
        fprintf(stderr, "Read error: %s\n", strerror(errno));
        quitting = 1;
        return;
    }

    buf_used += rv;

    /* Scan for newlines in the line buffer; we're careful here to deal with embedded \0s
     * a broken server may send, as well as only processing lines that are complete.
     */
    char *line_start = inbuf;
    char *line_end;
    while ((line_end = memchr(line_start, '\n', buf_used - (line_start - inbuf))))
    {
        *line_end = 0;
        /* Straight out of RFC */
        assert(strlen(line_start) <= MAX_IRC_BUF);
        printf(">> %s\n", line_start);
        parse_line(line_start);
        line_start = line_end + 1;
    }

    /* Shift buffer down so the unprocessed data is at the start */
    buf_used -= (line_start - inbuf);
    memmove(innbuf, line_start, buf_used);
}
