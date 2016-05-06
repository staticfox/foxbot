/*
 *   start.c -- May 1 2016 19:44:55 EST
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

#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <foxbot/foxbot.h>
#include <foxbot/socket.h>

#include "check_foxbot.h"
#include "check_server.h"

static pthread_t tid;

void
delete_foxbot(void)
{
    quit_foxbot();
}

void
new_foxbot(int port)
{
    char s_port[16];

    snprintf(s_port, sizeof(s_port), "%d", port);

    char *args[] = {
        PREFIX "/bin/foxbot",
        "-tp", s_port, "-c",
        TESTDIST "/foxbot.conf",
        NULL /* C standard requires argv[argc] == NULL */
    };

    init_foxbot(sizeof(args) / sizeof(*args) - 1, args);
    while (exec_foxbot() == BS_RUNNING);
}

int
new_testserver(void)
{
    int nport;
    int err;

    if((nport = setup_test_server()) < 0) {
        fprintf(stderr, "Unable to start socket server.\n");
        return -1;
    }

    err = pthread_create(&tid, NULL, start_listener, NULL);

    /* Brief moment to start the listener */
    struct timespec tim;
    tim.tv_sec  = 0;
    tim.tv_nsec = 900000L;
    nanosleep(&tim , NULL);

    if(err != 0) {
        fprintf(stderr, "Thread error: %s\n", strerror(errno));
        return -1;
    }

    return nport;
}

void
begin_test(void)
{
    int nport;
    nport = new_testserver();
    new_foxbot(nport);
}

void
end_test(void)
{
    delete_foxbot();
    tests_done = 1;
    shutdown_test_server();
}

static enum bot_status
io(void)
{
    const char *const line = io_simple_readline(&bot.io, "");
    if (!line)
        return BS_ERRORED;
    printf(">> %s\n", line);
    fflush(stdout);
    if (!parse_line(line))
        return BS_QUIT;
    return BS_RUNNING;
}

enum bot_status
write_and_wait(char *data)
{
    char buf[MAX_IRC_BUF];

    snprintf(buf, sizeof(buf), "%s\r\n", data);

    fox_write(buf);

    /* Enough to determine whether it will never hit
     * but not enough to timeout libcheck. */
    for (int i = 0; i < 900000; i++) {
        const enum bot_status status = io();
        if (status || (bot.msg->buffer &&
                       (strcmp(bot.msg->buffer, data) == 0)))
            return status;
    }

    fprintf(stderr, "READ FAILED: %s\n", data);
    ck_assert(0);
}

void
wait_for(char *line, ...)
{
    char buf[MAX_IRC_BUF] = {0};
    va_list ap;
    va_start(ap, line);
    vsnprintf(buf, MAX_IRC_BUF, line, ap);
    va_end(ap);

    for (int i = 0; i < 900000; i++) {
        if (strcmp(bot.msg->buffer, buf) == 0)
            return;
        io();
    }

    fprintf(stderr, "WAIT_FOR FAILED: %s\n", buf);
    ck_assert(0);
    return;
}

void
wait_for_command(enum commands cmd)
{
    for (int i = 0; i < 900000; i++) {
        if (bot.msg->ctype == cmd)
            return;
        io();
    }

    fprintf(stderr, "WAIT_FOR_COMMAND FAILED: %d\n", cmd);
    ck_assert(0);
    return;
}

void
wait_for_numeric(unsigned int numeric)
{
    for (int i = 0; i < 900000; i++) {
        if (bot.msg->numeric == numeric)
            return;
        io();
    }

    fprintf(stderr, "WAIT_FOR_NUMERIC FAILED: %d\n", numeric);
    ck_assert(0);
    return;
}

void
wait_for_last_buf(char *line, ...)
{
    char buf[MAX_IRC_BUF] = {0};
    va_list ap;
    va_start(ap, line);
    vsnprintf(buf, MAX_IRC_BUF, line, ap);
    va_end(ap);

    for (int i = 0; i < 100000000; i++) {
        if (strcmp(last_buffer, buf) == 0)
            return;
    }

    fprintf(stderr, "WAIT_FOR_LAST_BUF FAILED: %s\n", buf);
    ck_assert(0);
    return;
}
