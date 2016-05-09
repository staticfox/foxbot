/*
 *   irc_numeric.c -- May 7 2016 20:41:09 EDT
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

#include <foxbot/channel.h>
#include <foxbot/foxbot.h>
#include <foxbot/message.h>
#include <foxbot/memory.h>
#include <foxbot/user.h>

static unsigned int
modchar_to_bitflag(const char f)
{
    switch(f) {
    case '~': return CFLAG_OWNER;
    case '&': return CFLAG_ADMIN;
    case '@': return CFLAG_OPERATOR;
    case '%': return CFLAG_HALFOP;
    case '+': return CFLAG_VOICE;
    case '-': return CFLAG_USEROP;
    default:  return CFLAG_SPECIAL;
    }
}

static bool
is_channel_status_prefix(char prefix)
{
    for(size_t jj = 0; bot.ircd->supports.prefix[jj]; ++jj) {
        if (prefix == bot.ircd->supports.prefix[jj]) {
            /* If the mode isn't in this table, then we don't
             * know what to call it. Thanks InspIRCd :/ */
            return true;
        }
    }
    return false;
}

static void
set_flags(const char *flags, struct member_t *member)
{
    char flag_buf[MAX_IRC_BUF];
    char *ptr = flag_buf;

    member->user->ircop = false;
    member->opstatus = 0x0;

    for(size_t ii = 0; flags[ii]; ++ii) {
        /* Check if it's a channel status prefix. If it is, don't
         * count it as a user flag. */
        if (is_channel_status_prefix(flags[ii])) {
            member->opstatus |= modchar_to_bitflag(flags[ii]);
            continue;
        }

        switch(flags[ii]) {
        case 'G':
            member->user->away = true;
            break;
        case 'H':
            member->user->away = false;
            break;
        case '*':
            member->user->ircop = true;
            break;
        default:
            ptr += sprintf(ptr, "%c", flags[ii]);
            break;
        }
    }

    *ptr = 0;
    xfree(member->user->flags);
    if (strlen(flag_buf) > 0)
        member->user->flags = xstrdup(flag_buf);
    else
        member->user->flags = NULL;
}

void
parse_rpl_whoreply(void)
{
    char *token, *string, *tofree;
    char *ident = NULL, *host = NULL, *server = NULL;
    struct channel_t *channel;
    struct member_t *member;
    struct user_t *user;
    size_t length = 0;
    int i = 0;

    tofree = string = xstrdup(bot.msg->params);

    while ((token = fox_strsep(&string, " ")) != NULL) {
        switch(i) {
        case 0:
            channel = find_channel(token);
            if (!channel) {
                do_error("Received WHO for unknown channel %s.", token);
                goto done;
            }
            break;
        case 1:
            ident = xstrdup(token);
            break;
        case 2:
            host = xstrdup(token);
            break;
        case 3:
            server = xstrdup(token);
            break;
        case 4:
            user = find_nick(token);
            if (!user)
                user = make_nuh(token, ident, host);
            xfree(user->server);
            user->server = xstrdup(server);
            member = channel_get_membership(channel, user);
            if (!member)
                member = add_user_to_channel(channel, user);
            break;
        case 5:
            set_flags(token, member);
            break;
        case 6:
            user->hops = atoi(token + 1);
            break;
        default:
            xfree(member->user->gecos);
            member->user->gecos = NULL;
            if (bot.msg->params[length])
                member->user->gecos = xstrdup(bot.msg->params + length);
            goto done;
            break;
        }
        length += strlen(token) + 1;
        i++;
    }

done:
    xfree(tofree);
    xfree(ident);
    xfree(host);
    xfree(server);
}

void
parse_rpl_whospcrpl(void)
{
    char *token, *string, *tofree;
    char *ident = NULL, *host = NULL, *server = NULL;
    struct channel_t *channel;
    struct member_t *member;
    struct user_t *user;
    size_t length = 0;
    int i = 0;

    tofree = string = xstrdup(bot.msg->params);

    while ((token = fox_strsep(&string, " ")) != NULL) {
        switch(i) {
        case 0:
            channel = find_channel(token);
            if (!channel) {
                do_error("Received WHO for unknown channel %s.", token);
                goto done;
            }
            break;
        case 1:
            ident = xstrdup(token);
            break;
        case 2:
            host = xstrdup(token);
            break;
        case 3:
            server = xstrdup(token);
            break;
        case 4:
            user = find_nick(token);
            if (!user)
                user = make_nuh(token, ident, host);
            xfree(user->server);
            user->server = xstrdup(server);
            member = channel_get_membership(channel, user);
            if (!member)
                member = add_user_to_channel(channel, user);
            break;
        case 5:
            set_flags(token, member);
            break;
        case 6:
            member->user->hops = atoi(token);
            break;
        case 7:
            member->user->idle = atoi(token);
            break;
        case 8:
            xfree(member->user->account);
            if (strcmp(token, "0") == 0)
                member->user->account = NULL;
            else
                member->user->account = xstrdup(token);
            break;
        default:
            xfree(member->user->gecos);
            member->user->gecos = NULL;
            if (bot.msg->params[length] && bot.msg->params[length + 1])
                member->user->gecos = xstrdup(bot.msg->params + length + 1);
            goto done;
            break;
        }
        length += strlen(token) + 1;
        i++;
    }

done:
    xfree(tofree);
    xfree(ident);
    xfree(host);
    xfree(server);
}
