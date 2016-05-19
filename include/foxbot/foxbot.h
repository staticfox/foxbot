/*
 *   foxbot.h -- April 27 2016 13:38:43 EDT
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

#ifndef FOX_FOXBOT_H_
#define FOX_FOXBOT_H_

#include "ircd.h"
#include "socket.h"

#define MAX_IRC_BUF 512
#include <signal.h>
#include <stdbool.h>

#define MAX_IRC_BUF 512
#define MAX_IO_BUF 2048

/* Runtime flags */
#define RUNTIME_RUN     0x01
#define RUNTIME_TEST    0x02

enum commands {
    PRIVMSG,
    NOTICE,
    PING,
    PONG,
    JOIN,
    PART,
    QUIT,
    MODE,
    KICK,
    ERROR,
    NUMERIC,
    NICK,
    CAP,
    ACCOUNT
};

enum bot_status {
    BS_RUNNING,
    BS_QUIT,
    BS_PAUSED,                          /* only occurs during testing */
    BS_ERRORED
};

struct bot_t {
    io_state io;
    int flags;
    int test_port;
    bool registered;
    bool modes[256];
    bool nonblocking;
    const char *quit_reason;
    struct msg_t *msg;
    struct user_t *user;
    struct addrinfo *hil; /* Needed for reconnects */
    struct ircd_t *ircd;
};

bool is_registered(void);
void raw(char *fmt, ...);
void privmsg(const char *target, char *message, ...);
void join(const char *channel);
void join_with_key(const char *channel, const char *key);
void do_quit(const char *message);
void do_error(char *line, ...);
void init_foxbot(int argc, char **argv);
enum bot_status exec_foxbot(void);
void quit_foxbot(void);

extern struct bot_t bot;

#endif
