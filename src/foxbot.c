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

#include <config.h>

#include <ctype.h>
#include <getopt.h>
#include <stdarg.h>
#include <string.h>

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

    if (bot.registered && botconfig.debug_channel)
        privmsg(botconfig.debug_channel, buf);
    else
        fprintf(stderr, "%s\n", buf);
}

void
foxbot_quit(void)
{
    quitting = 1;
}

static void
parse_opts(int argc, char **argv)
{
    bool got_port = false;
    for (int c = 0; (c = getopt(argc, argv, "c:htvp:")) != -1; ) {
        switch (c) {
        case 'c':
            xfree(conf_parser_ctx.config_file_path);
            conf_parser_ctx.config_file_path = xstrdup(optarg);
            break;
        case 'h':
            printf("Help coming soon.\n");
            exit(EXIT_SUCCESS);
            break; /* shut up compiler */
        case 'p':
            bot.test_port = atoi(optarg);
            got_port = true;
            break;
        case 't':
            bot.flags |= RUNTIME_TEST;
            break;
        case 'v':
            printf("foxbot version 0.0.1\n");
            exit(EXIT_SUCCESS);
            break;
        case '?':
            if (optopt == 'p')
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            exit(EXIT_SUCCESS);
            break;
        default:
            printf("Invalid Option: -%c\n", c);
            break;
        }
    }

    if (!got_port)
        bot.test_port = -1;
}

int
main_foxbot(int argc, char **argv)
{
    static const struct msg_t MSG_EMPTY;
    bot.msg = xmalloc(sizeof(*bot.msg));
    *bot.msg = MSG_EMPTY;
    bot.msg->from = xmalloc(sizeof(*bot.msg->from));
    bot.ircd = xmalloc(sizeof(*bot.ircd));
    bot.flags = RUNTIME_RUN;

    parse_opts(argc, argv);
    read_conf_file();
    setup_signals();
    create_and_bind();
    establish_link();

    while (!quitting) {
        if ((bot.flags & RUNTIME_TEST)
                && bot.msg->command
                && (strcmp(bot.msg->command, "FOXBOT") == 0))
            break;

        io();
    }

    if (bot.registered && !(bot.flags & RUNTIME_TEST)) {
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
