/*
 *   message.c -- April 29 2016 10:07:07 EDT
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

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include <foxbot/cap.h>
#include <foxbot/conf.h>
#include <foxbot/foxbot.h>
#include <foxbot/hook.h>
#include <foxbot/irc_literal.h>
#include <foxbot/irc_numeric.h>
#include <foxbot/ircd.h>
#include <foxbot/memory.h>
#include <foxbot/message.h>
#include <foxbot/user.h>

static int
do_set_enum(const char *command, enum commands ecmd)
{
    if (strcmp(bot.msg->command, command) == 0) {
        bot.msg->ctype = ecmd;
        return 1;
    }

    return 0;
}

/* Do the messy string compare stuff now, that way
 * later on all we can check for is the ctype. */
static void
set_command_enum(void)
{
    assert(bot.msg->command);

    size_t n = strlen(bot.msg->command);
    unsigned int digits = 0;

    for (size_t ii = 0; bot.msg->command[ii]; ii++)
        if (isdigit(bot.msg->command[ii]))
            digits++;

    if (digits == n) {
        bot.msg->ctype = NUMERIC;
        bot.msg->numeric = atoi(bot.msg->command);
        return;
    }

    if (do_set_enum("PRIVMSG", PRIVMSG)) return;
    if (do_set_enum("NOTICE",  NOTICE) ) return;
    if (do_set_enum("PING",    PING)   ) return;
    if (do_set_enum("PONG",    PONG)   ) return;
    if (do_set_enum("JOIN",    JOIN)   ) return;
    if (do_set_enum("PART",    PART)   ) return;
    if (do_set_enum("QUIT",    QUIT)   ) return;
    if (do_set_enum("MODE",    MODE)   ) return;
    if (do_set_enum("KICK",    KICK)   ) return;
    if (do_set_enum("ERROR",   ERROR)  ) return;
    if (do_set_enum("NICK",    NICK)   ) return;
    if (do_set_enum("CAP",     CAP)    ) return;
    if (do_set_enum("ACCOUNT", ACCOUNT)) return;

    do_error("Unhandled command: %s", bot.msg->command);
}

static void
reset_message(void)
{
    static const struct msg_t EMPTY_MESSAGE;
    xfree(bot.msg->buffer);
    xfree(bot.msg->source);
    xfree(bot.msg->command);
    xfree(bot.msg->target);
    xfree(bot.msg->params);
    *bot.msg = EMPTY_MESSAGE;
}

void
hook_numeric(void)
{
    switch(bot.msg->numeric) {
    case   1: /* RPL_WELCOME */
        parse_rpl_welcome();
        break;
    case   4: /* RPL_MYINFO */
        parse_rpl_myinfo();
        break;
    case   5: /* RPL_ISUPPORT */
        parse_rpl_isupport();
        break;
    case 352: /* RPL_WHOREPLY */
        parse_rpl_whoreply();
        break;
    case 354: /* RPL_WHOSPCRPL */
        parse_rpl_whospcrpl();
        break;
    case 421: /* ERR_UNKNOWNCOMMAND */
        do_error(bot.msg->buffer);
        break;
    default:
        break;
    }
    /* Too many numerics, let the plugin figure
     * it out. */
    exec_hook("on_numeric");
}
}

void
hook_literal(void)
{
    switch(bot.msg->ctype) {
    case JOIN:
        handle_join();
        exec_hook("on_join");
        break;
    case PART:
        handle_part();
        exec_hook("on_part");
        break;
    case KICK:
        handle_kick();
        exec_hook("on_kick");
        break;
    case MODE:
        handle_mode();
        exec_hook("on_mode");
        break;
    case QUIT:
        handle_quit();
        exec_hook("on_quit");
        break;
    case NICK:
        handle_nick();
        exec_hook("on_nick");
        break;
    case CAP:
        handle_cap();
        exec_hook("on_cap");
        break;
    case ACCOUNT:
        handle_account();
        exec_hook("on_account");
        break;
    case PRIVMSG:
        exec_hook("on_privmsg");
        break;
    case NOTICE:
        exec_hook("on_notice");
        break;
    default:
        break;
    }
}

void
call_hooks(void)
{
    if (bot.msg->ctype == NUMERIC)
        hook_numeric();
    else
        hook_literal();
}

char *
fox_strsep(char **stringp, const char *delim)
{
    char *p = *stringp;
    if (p) {
        char *q = p + strcspn(p, delim);
        *stringp = *q ? q + 1 : NULL;
        *q = '\0';
    }
    return p;
}

bool
parse_line(const char *line)
{
    bool quitting = false;
    unsigned int i = 0;
    int params = 1;
    char *token, *string, *tofree;

    reset_message();

    bot.msg->buffer = xstrdup(line);

    for (size_t ii = 0; line[ii]; ii++)
        if (line[ii] == ' ')
            params++;

    tofree = string = xstrdup(line);

    while (((token = fox_strsep(&string, " ")) != NULL) && i < 3) {
        if (params < 3) {
            if (i == 0 && strncmp(token, "PING", 4) == 0) {
                size_t n = strlen(line);
                char tmp[MAX_IRC_BUF];
                memcpy(tmp, line, n);
                bot.msg->ctype = PING;
                tmp[1] = 'O';
                tmp[n] = '\n';
                tmp[n + 1] = '\0';
                raw(tmp);
                exec_hook("on_ping");
                goto end;
            } else {
                bot.msg->is_invalid = true;
                do_error("(2) Invalid line detected. Skipping... %s", line);
                goto end;
            }
        }

        switch(i) {
        case 0:
            if (token[0] == ':') {
                bot.msg->source = xstrdup(token + 1);
                struct user_t *user = get_nuh(token + 1);
                if (user == NULL)
                    bot.msg->from_server = true;
                else
                    bot.msg->from = user;
            } else {
                if (strcmp(token, "ERROR") == 0) {
                    quitting = true;
                    exec_hook("on_error");
                    goto end;
                }
                bot.msg->is_invalid = true;
                do_error("Invalid line detected. Skipping... %s", line);
                goto end;
            }
            break;
        case 1:
            bot.msg->command = xstrdup(token);
            set_command_enum();
            break;
        case 2:
            bot.msg->target = xstrdup(token);
            break;
        }

        i++;
    }

    if (params > 2) {
        /* Messy, gets the parameters from the IRCd command by setting our
         * params to start at the point after the source, spaces, command, etc.
         */
        size_t len = strlen(bot.msg->source + 1) + 1 + strlen(bot.msg->command) + 1 + strlen(bot.msg->target) + 3;
        if (strlen(line) > len)
            bot.msg->params = xstrdup(line + len);
    }

    call_hooks();

end:
    xfree(tofree);

    /* Incase we get a message from an unknown user */
    if (!bot.msg->from_server
            && bot.msg->from
            && bot.msg->from != bot.user
            && bot.msg->from->number_of_channels == 0)
        delete_user(bot.msg->from);
    return !quitting;
}
