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
#include <foxbot/socket.h>

int
create_and_bind(void)
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

    bot.fd = socket(bot.hil->ai_family, bot.hil->ai_socktype, bot.hil->ai_protocol);
    if (bot.fd == -1) {
        do_error(strerror(errno));
        exit(EXIT_FAILURE);
    }

    return 0;
}

int
establish_link(void)
{
    int status;

    status = connect(bot.fd, bot.hil->ai_addr, bot.hil->ai_addrlen);
    if (status == -1) {
        do_error(strerror(errno));
        exit(EXIT_FAILURE);
    }

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
    char sbuf[MAX_IRC_BUF] = {0};
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
    static char inbuf[MAX_IRC_BUF];
    static size_t buf_used;
    size_t buf_remain = sizeof(inbuf) - buf_used;

    if (buf_remain == 0) {
        fprintf(stderr, "Line exceeded buffer length\n");
        quitting = 1;
        return;
    }

    ssize_t rv = recv(bot.fd, inbuf + buf_used, buf_remain, 0);

    if (rv == 0) {
        fprintf(stderr, "Remote host closed the connection\n");
        quitting = 1;
        return;
    }

    /* no data for now, call back when the socket is readable */
    if (rv < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return;

    if (rv < 0) {
        if (errno != EINTR)
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
    while ((line_end = memchr(line_start, '\n', buf_used))) {
        *line_end = '\0';
        if (strlen(line_start) > 0 && line_end[-1] == '\r')
            line_end[-1] = '\0';
        /* Straight out of RFC */
        assert(strlen(line_start) <= MAX_IRC_BUF);
        printf(">> %s\n", line_start);
        parse_line(line_start);
        ++line_end;
        buf_used -= line_end - line_start;
        line_start = line_end;
    }

    /* Shift buffer down so the unprocessed data is at the start */
    memmove(inbuf, line_start, buf_used);
}
