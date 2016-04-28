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

struct user_t {
    char *nick;
    char *ident;
    char *host;
};

struct msg_t {
    enum commands ctype;
    unsigned int numeric;
    char *buffer;
    struct user_t *from;
    bool from_server;
    char *source;
    char *command;
    char *target;
    char *params;
    bool is_invalid;
};

struct bot_t {
    int fd;
    bool registered;
    struct msg_t *msg;
    struct addrinfo *hil; /* Needed for reconnects */
};

bool is_registered(void);
void raw(char *fmt, ...);
void privmsg(const char *target, const char *message);
void join(const char *channel);
void do_quit(const char *message);
void do_error(char *line, ...);

extern bool quitting;
extern struct bot_t bot;
