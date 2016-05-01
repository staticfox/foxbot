/*
 *   message.c -- April 29 2016 10:07:07 EST
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
#include <ctype.h>
#include <string.h>

#include "conf.h"
#include "foxbot.h"
#include "irc.h"
#include "ircd.h"
#include "memory.h"
#include "message.h"
#include "user.h"

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

    size_t ii;
    size_t n = strlen(bot.msg->command);
    unsigned int digits = 0;

    for (ii = 0; bot.msg->command[ii]; ii++)
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

    do_error("Unhandled command: %s", bot.msg->command);
}

static void
reset_message(void)
{
    xfree(bot.msg->buffer);
    bot.msg->buffer = NULL;

    xfree(bot.msg->source);
    bot.msg->source = NULL;

    xfree(bot.msg->command);
    bot.msg->command = NULL;

    xfree(bot.msg->target);
    bot.msg->target = NULL;

    xfree(bot.msg->params);
    bot.msg->params = NULL;

    bot.msg->from_server = false;
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
    case 421: /* ERR_UNKNOWNCOMMAND */
        do_error(bot.msg->buffer);
        break;
    default:
        break;
    }
}

void
hook_literal(void)
{
    switch(bot.msg->ctype) {
    case QUIT:
        handle_quit();
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

void
parse_line(const char *line)
{
    unsigned int i = 0, ii;
    int params = 1;
    char *token, *string, *tofree;

    reset_message();

    bot.msg->buffer = xstrdup(line);

    for (ii = 0; line[ii] != '\0'; ii++)
        if (line[ii] == ' ')
            params++;

    tofree = string = xstrdup(line);

    while (((token = fox_strsep(&string, " ")) != NULL) && i < 3) {
        if (params < 3) {
            if (i == 0 && strncmp(token, "PING", 4) == 0) {
                size_t n = strlen(line);
                char tmp[MAX_IRC_BUF];
                memcpy(tmp, line, n);
                tmp[1] = 'O';
                tmp[n] = '\n';
                tmp[n + 1] = '\0';
                raw(tmp);
                goto end;
            } else {
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
                    quitting = 1;
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
        else if (bot.msg->ctype != JOIN && bot.msg->ctype != PART)
            do_error("Received command with no arguments: (%zu) > %zu - Command: %s\n", strlen(line), len, bot.msg->command);
    }

    call_hooks();

end:
    xfree(tofree);
}
