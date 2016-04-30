/*
 *   ircd.c -- April 28 2016 19:42:29 EST
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

#include <stdio.h>
#include <string.h>

#include "conf.h"
#include "memory.h"
#include "message.h"
#include "foxbot.h"
#include "user.h"

/* 001 | RPL_WELCOME */
void
parse_rpl_welcome(void)
{
    bot.registered = true;
    join(botconfig.channel);

    /* That's me! */
    make_me(bot.msg->target);
}

/* 004 | RPL_MYINFO */
void
parse_rpl_myinfo(void)
{
    int i = 0;
    char *token, *string, *tofree;

    tofree = string = xstrdup(bot.msg->params);

    while (((token = fox_strsep(&string, " ")) != NULL) && i < 3) {
        switch(i) {
        case 0:
            bot.ircd->name = xstrdup(token);
            break;
        case 1:
            bot.ircd->version = xstrdup(token);
            break;
        case 2:
            bot.ircd->user_modes = xstrdup(token);
            break;
        }
        i++;
    }

    xfree(tofree);
}

/* 005 | RPL_ISUPPORT */
void
parse_rpl_isupport(void)
{
    char *token, *string, *tofree;

    tofree = string = xstrdup(bot.msg->params);

#define SUPPORT_BOOL(x, y, z) if (strcmp(x, y) == 0) { z = true; continue; }
    while ((token = fox_strsep(&string, " ")) != NULL) {
        /* End of anything useful to parse */
        if (strcmp(token, ":are") == 0)
            break;

        SUPPORT_BOOL(token, "EXCEPTS", bot.ircd->supports.excepts);
        SUPPORT_BOOL(token, "INVEX", bot.ircd->supports.invex);
        SUPPORT_BOOL(token, "KNOCK", bot.ircd->supports.knock);
        SUPPORT_BOOL(token, "WHOX", bot.ircd->supports.whox);

        if (strncmp(token, "NETWORK=", 8) == 0) {
            bot.ircd->network = xstrdup(token + 8);
            continue;
        }

        if (strncmp(token, "CHANTYPES=", 10) == 0) {
            bot.ircd->supports.chan_types = xstrdup(token + 10);
            continue;
        }

        if (strncmp(token, "PREFIX=", 7) == 0) {
            char *value = token + 7;
            char modes[MAX_IRC_BUF] = {0}, prefixes[MAX_IRC_BUF] = {0};
            size_t ii, pos = 0;
            bool mode = true;

            for (ii = 0; value[ii] != '\0'; ii++) {
                switch(value[ii]) {
                case '(': continue;
                case ')':
                    mode = false;
                    pos = 0;
                    continue;
                default:
                    if (mode)
                        modes[pos++] = value[ii];
                    else
                        prefixes[pos++] = value[ii];
                    continue;
                }
            }

            bot.ircd->supports.chanop_modes = xstrdup(modes);
            bot.ircd->supports.prefix = xstrdup(prefixes);
            continue;
        }

        if (strncmp(token, "NICKLEN=", 8) == 0) {
            bot.ircd->nick_length = atoi(token + 8);
            continue;
        }

        if (strncmp(token, "CHANNELLEN=", 11) == 0) {
            bot.ircd->channel_length = atoi(token + 11);
            continue;
        }

        if (strncmp(token, "CHANLIMIT=", 10) == 0) {
            char *value = token + 10;
            value = strtok(value, ":");
            bot.ircd->chan_limit = atoi(strtok(NULL, ":"));
            continue;
        }

        if (strncmp(token, "TOPICLEN=", 9) == 0) {
            bot.ircd->topic_length = atoi(token + 9);
            continue;
        }
    }
#undef SUPPORT_BOOL

    xfree(tofree);
}
