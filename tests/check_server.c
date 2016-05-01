/*
 *   check_server.c -- May 1 2016 10:58:00 EST
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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "foxbot.h"
#include "check_server.h"

int tests_done = 0;

/* Should we even bother parsing? */
static void
parse_buffer(const char *buf)
{
    (void)buf;
}

void
fox_write(int fd, char *line, ...)
{
    char buf[MAX_IO_BUF] = {0};
    ssize_t writeval;
    int n;

    va_list ap;
    va_start(ap, line);
    vsnprintf(buf, MAX_IO_BUF, line, ap);
    va_end(ap);

    writeval = strlen(buf);

    n = write(fd, buf, writeval);

    if (n < 0) {
       fprintf(stderr, "Write error: %s\n", strerror(errno));
       exit(EXIT_FAILURE);
    }
}

/* Basically copy pasted from socket.c. To understand
 * the implementation better, check the comments in
 * the main socket.c file as well as the git revision
 * history. This is also pretty un-necessary, considering
 * we are physically controlling what gets sent so why we
 * need to have a complicated read system instead of just
 * piping it out to /dev/null is beyond me...
 */
void
fox_read(int fd)
{
    static char inbuf[MAX_IRC_BUF];
    static size_t buf_used;
    size_t buf_remain = sizeof(inbuf) - buf_used;

    if (buf_remain == 0) {
        fprintf(stderr, "Line exceeded buffer length\n");
        tests_done = 1;
        return;
    }

    ssize_t rv = recv(fd, inbuf + buf_used, buf_remain, 0);

    if (rv == 0) {
        fprintf(stderr, "Remote host closed the connection\n");
        tests_done = 1;
        return;
    }

    if (rv < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return;

    if (rv < 0) {
        fprintf(stderr, "Read error: %s\n", strerror(errno));
        tests_done = 1;
        return;
    }

    buf_used += rv;

    char *line_start = inbuf;
    char *line_end;
    while ((line_end = memchr(line_start, '\n', buf_used))) {
        *line_end = '\0';

        if (strlen(line_start) > 0 && line_end[-1] == '\r')
            line_end[-1] = '\0';

        assert(strlen(line_start) <= MAX_IRC_BUF);
        printf(">> %s\n", line_start);
        parse_buffer(line_start);
        ++line_end;
        buf_used -= line_end - line_start;
        line_start = line_end;
    }

    memmove(inbuf, line_start, buf_used);
}

/* We should globally declare this fd since
 * it's going to be important. */
void
start_listener(int fd)
{
    int newsockfd;
    struct sockaddr_in cli_addr;
    socklen_t clilen;

    memset(&cli_addr, 0, sizeof(cli_addr));

    listen(fd, 5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(fd, (struct sockaddr *)&cli_addr, &clilen);

    if (newsockfd < 0) {
       fprintf(stderr, "Accept error: %s\n", strerror(errno));
       exit(EXIT_FAILURE);
    }

    while(!tests_done) {
       fox_read(newsockfd);
    }

    close(newsockfd);
}

void
shutdown_test_server(int fd)
{
    close(fd);
}

int
setup_test_server(void)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
       fprintf(stderr, "Socket error: %s", strerror(errno));
       return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(43254);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Bind error: %s", strerror(errno));
        return -1;
    }

    return sockfd;
}
