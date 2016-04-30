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

#include <limits.h>
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

static bool parse_long(const char *str, long *value)
{
    char *end;
    const long n = strtol(str, &end, 10);
    return str != end && (*value = n);
}

static bool parse_uint(const char *str, unsigned *value)
{
    long n;
    return parse_long(str, &n) && n >= 0 && n <= UINT_MAX && (*value = n);
}

static bool strip_prefix(const char *str,
                         const char *prefix,
                         char **suffix /* nullable */)
{
    for (; *prefix; ++str, ++prefix)
        if (*str != *prefix)
            return false;
    if (suffix)
        *suffix = (char *)str;
    return true;
}

static bool support_bool(const char *token, const char *word, bool *value)
{
    return !strcmp(token, word) && (*value = true);
}

static bool support_str(const char *token, const char *prefix, char **value)
{
    char *suffix;
    return strip_prefix(token, prefix, &suffix) && (*value = xstrdup(suffix));
}

static bool support_uint(const char *token, const char *prefix, unsigned *value)
{
    char *suffix;
    if (!strip_prefix(token, prefix, &suffix))
        return false;
    if (!parse_uint(suffix, value)) {
        fprintf(stderr, "warning: not a valid unsigned int: %s\n", suffix);
        return false;
    }
    return true;
}

/* 005 | RPL_ISUPPORT */
void
parse_rpl_isupport(void)
{
    char *token, *string, *tofree;

    tofree = string = xstrdup(bot.msg->params);

    while ((token = fox_strsep(&string, " ")) != NULL) {
        /* End of anything useful to parse */
        if (strcmp(token, ":are") == 0)
            break;

        if (support_bool(token, "EXCEPTS", &bot.ircd->supports.excepts)) continue;
        if (support_bool(token, "INVEX", &bot.ircd->supports.invex)) continue;
        if (support_bool(token, "KNOCK", &bot.ircd->supports.knock)) continue;
        if (support_bool(token, "WHOX", &bot.ircd->supports.whox)) continue;

        if (support_str(token, "NETWORK=", &bot.ircd->network)) continue;
        if (support_str(token, "CHANTYPES=", &bot.ircd->supports.chan_types)) continue;

        if (support_uint(token, "NICKLEN=", &bot.ircd->nick_length)) continue;
        if (support_uint(token, "CHANNELLEN=", &bot.ircd->channel_length)) continue;
        if (support_uint(token, "TOPICLEN=", &bot.ircd->topic_length)) continue;

        char *value;

        if (strip_prefix(token, "CHANLIMIT=", &value)) {
            strtok(value, ":");
            bot.ircd->chan_limit = atoi(strtok(NULL, ":"));
            continue;
        }

        if (strip_prefix(token, "PREFIX=", &value)) {
            char modes[MAX_IRC_BUF] = {0}, prefixes[MAX_IRC_BUF] = {0};
            bool mode = true;

            for (size_t ii = 0, pos = 0; value[ii] != '\0'; ii++) {
                switch(value[ii]) {
                case '(':
                    continue;
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
    }

    xfree(tofree);
}
