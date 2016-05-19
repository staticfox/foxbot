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
#include <foxbot/hook.h>
#include <foxbot/ircd.h>
#include <foxbot/list.h>
#include <foxbot/message.h>
#include <foxbot/module.h>
#include <foxbot/signal.h>
#include <foxbot/socket.h>
#include <foxbot/user.h>

struct bot_t bot; /* That's me */

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

void
init_foxbot(int argc, char **argv)
{
    static const struct msg_t MSG_EMPTY;
    static const struct ircd_t IRCD_EMPTY;
    bot.msg = xmalloc(sizeof(*bot.msg));
    *bot.msg = MSG_EMPTY;
    bot.msg->from = xmalloc(sizeof(*bot.msg->from));
    bot.ircd = xmalloc(sizeof(*bot.ircd));
    *bot.ircd = IRCD_EMPTY;
    bot.flags = RUNTIME_RUN;

    parse_opts(argc, argv);
    read_conf_file();
    setup_signals();
    create_socket();
    establish_link();
    load_module("echo_test.so");
}

void
quit_foxbot(void)
{
    if (bot.registered) {
        char buf[MAX_IRC_BUF];
        const char *reason = bot.quit_reason;
        if (!reason)
            reason = "QUIT";
        snprintf(buf, sizeof(buf), "Exiting due to %s", reason);
        do_quit(buf);
    }

    clear_channels();
    destroy_socket();

    if (bot.msg)
        xfree(bot.msg->from);
    xfree(bot.msg);
    xfree(bot.ircd);
    static const struct bot_t BOT_EMPTY;
    bot = BOT_EMPTY;
    quit_signal = 0;
}

static bool
is_pause_message(const char *line)
{
    return line[0] == '#';
}

enum bot_status
exec_foxbot(void)
{
    if (quit_signal) {
        bot.quit_reason = "SIGINT";
        return BS_QUIT;
    }
    const char *const line = io_simple_readline(&bot.io, "");
    if (!line)
        return BS_ERRORED;
    printf(">> %s\n", line);
    fflush(stdout);
    if ((bot.flags & RUNTIME_TEST) && is_pause_message(line)) {
        if (line[1])
            raw("%s\n", line + 1);
        return BS_PAUSED;
    }
    if (!parse_line(line))
        return BS_QUIT;
    return BS_RUNNING;
}
