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
    SPAM_DEBUG
    snprintf(s_port, sizeof(s_port), "%d", port);
    SPAM_DEBUG
    char *args[] = {
        PREFIX "/bin/foxbot",
        "-tp", s_port, "-c",
        TESTDIST "/foxbot.conf",
        NULL /* C standard requires argv[argc] == NULL */
    };
    SPAM_DEBUG
    init_foxbot(sizeof(args) / sizeof(*args) - 1, args);
    SPAM_DEBUG
    yield_to_bot();
    SPAM_DEBUG
}

int
new_testserver(void)
{
    int nport;

    if((nport = setup_test_server()) < 0) {
        fprintf(stderr, "Unable to start socket server.\n");
        return -1;
    }

    if (pthread_create(&tid, NULL, start_listener, NULL)) {
        fprintf(stderr, "Thread error: %s\n", strerror(errno));
        return -1;
    }

    return nport;
}

static void
delete_testserver(void)
{
    fox_shutdown();
    if (pthread_join(tid, NULL)) {
        fprintf(stderr, "Cannot join server thread: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    shutdown_test_server();
}

void
begin_test(void)
{
    int nport;
    SPAM_DEBUG
    nport = new_testserver();
    SPAM_DEBUG
    new_foxbot(nport);
    SPAM_DEBUG
    yield_to_server();
    SPAM_DEBUG
    do_burst();
    SPAM_DEBUG
    /* Need to do this a few times to process the channel joins */
    yield_to_server();
    SPAM_DEBUG
    yield_to_server();
    SPAM_DEBUG
}

void
end_test(void)
{
    delete_foxbot();
    delete_testserver();
}

/** Tell the bot to pause itself.  When the bot receives that message, it will
  * return control to the caller.  If the `msg` is not an empty string, the
  * provided message is also sent to the server.  Generally, `send_pause`
  * should always be paired with `exec_bot`. */
static void
send_pause(const char *msg)
{
    SPAM_DEBUG
    fox_write("#%s\r\n", msg);
    SPAM_DEBUG
}

static enum bot_status
exec_bot(void)
{
    enum bot_status status;
    while ((status = exec_foxbot()) == BS_RUNNING);
    return status;
}

enum bot_status
yield_to_bot(void)
{
    SPAM_DEBUG
    send_pause("");
    SPAM_DEBUG
    return exec_bot();
}

void
yield_to_server(void)
{
    send_pause("#");
    SPAM_DEBUG
    /* Unlike yield_to_bot, we force the bot to run even if it wants to
       quit otherwise wait_for_server_notification could get stuck */
    while (exec_bot() != BS_PAUSED);
    SPAM_DEBUG
    wait_for_server_notification();
    SPAM_DEBUG
}

enum bot_status
write_and_wait(const char *data)
{
    fox_write("%s\r\n", data);
    const enum bot_status status = yield_to_bot();
    ck_assert_ptr_ne(bot.msg->buffer, NULL);
    ck_assert_str_eq(bot.msg->buffer, data);
    return status;
}

void
wait_for(const char *line, ...)
{
    char buf[MAX_IRC_BUF] = {0};
    va_list ap;
    va_start(ap, line);
    vsnprintf(buf, MAX_IRC_BUF, line, ap);
    va_end(ap);

    yield_to_server();
    ck_assert_ptr_ne(bot.msg->buffer, NULL);
    ck_assert_str_eq(bot.msg->buffer, buf);
}

void
wait_for_command(enum commands cmd)
{
    yield_to_server();
    ck_assert_int_eq(bot.msg->ctype, cmd);
}

void
wait_for_numeric(unsigned int numeric)
{
    yield_to_server();
    ck_assert_uint_eq(bot.msg->numeric, numeric);
}

void
wait_for_last_buf(const char *line, ...)
{
    char buf[MAX_IRC_BUF] = {0};
    va_list ap;
    va_start(ap, line);
    vsnprintf(buf, MAX_IRC_BUF, line, ap);
    va_end(ap);

    yield_to_server();
    ck_assert_str_eq(last_buffer, buf);
}
