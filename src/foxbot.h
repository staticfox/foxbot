/*
 *   foxbot.h -- April 27 2016 13:38:43 EST
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

#include <signal.h>
#include <stdbool.h>

#define MAX_IRC_BUF 512
#define MAX_IO_BUF 2048

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
};

struct bot_t {
    int fd;
    bool registered;
    struct msg_t *msg;
    struct addrinfo *hil; /* Needed for reconnects */
    struct ircd_t *ircd;
};

bool is_registered(void);
void raw(char *fmt, ...);
void privmsg(const char *target, const char *message);
void join(const char *channel);
void do_quit(const char *message);
void do_error(char *line, ...);
void parse_line(const char *line);

extern volatile sig_atomic_t quitting;
extern struct bot_t bot;

#endif
