/*
 *   foxbot.c -- April 26 2016 14:31:24 EST
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

#include <stdarg.h>

#include <foxbot/channel.h>
#include <foxbot/conf.h>
#include <foxbot/foxbot.h>
#include <foxbot/ircd.h>
#include <foxbot/list.h>
#include <foxbot/message.h>
#include <foxbot/signal.h>
#include <foxbot/socket.h>
#include <foxbot/user.h>

struct bot_t bot; /* That's me */
volatile sig_atomic_t quitting;

/* External helper functions */
bool
is_registered(void)
{
    return bot.registered;
}

/* IRC related socket helpers */
void
privmsg(const char *target, const char *message)
{
    char buf[MAX_IRC_BUF];
    snprintf(buf, sizeof(buf), "PRIVMSG %s :%s\n", target, message);
    raw(buf);
}

void
join(const char *channel)
{
    char buf[MAX_IRC_BUF];
    snprintf(buf, sizeof(buf), "JOIN %s\n", channel);
    raw(buf);
}

void
do_quit(const char *message)
{
    char buf[MAX_IRC_BUF];
    snprintf(buf, sizeof(buf), "QUIT :%s\n", message);
    raw(buf);
}

void
do_error(char *line, ...)
{
    char buf[MAX_IO_BUF] = { 0 };
    va_list ap;
    va_start(ap, line);
    vsnprintf(buf, MAX_IO_BUF, line, ap);
    va_end(ap);

    if (bot.registered)
        privmsg(botconfig.channel, buf);
    else
        fprintf(stderr, "%s\n", buf);
}

void
foxbot_quit(void)
{
    quitting = 1;
}

int
main_foxbot(/*int argc, char **argv*/)
{
    static const struct msg_t empty_msg;
    bot.msg = xmalloc(sizeof(*bot.msg));
    bot.msg->from = xmalloc(sizeof(*bot.msg->from));
    bot.ircd = xmalloc(sizeof(*bot.ircd));
    *bot.msg = empty_msg;

    init_channels();
    init_users();
    read_conf_file();
    setup_signals();
    create_and_bind();
    establish_link();

    while (!quitting) {
        io();
    }

    if (is_registered()) {
        char buf[MAX_IRC_BUF];
        const char *reason = "<unknown>";
        switch (quitting) {
        case 1:
            reason = "QUIT";
            break;
        case 2:
            reason = "SIGINT";
            break;
        }
        snprintf(buf, sizeof(buf), "Exiting due to %s", reason);
        do_quit(buf);
    }

    return 0;
}
