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
#include <foxbot/parser.h>
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
            *ptr++ = flags[ii];
        }
    }

    *ptr = '\0';
    xfree(member->user->flags);
    if (strlen(flag_buf) > 0)
        member->user->flags = xstrdup(flag_buf);
    else
        member->user->flags = NULL;
}

static bool
iparse_channel_user(char **string,
                    struct user_t **user_out,
                    struct member_t **member_out)
{
    char *channel_name, *ident, *host, *server, *nick, *flags;
    struct channel_t *channel;
    struct user_t *user;
    struct member_t *member;

    if (!(channel_name = fox_strsep(string, " ")))
        return false;
    if (!(channel = find_channel(channel_name))) {
        do_error("Received WHO for unknown channel %s.", channel_name);
        return false;
    }

    if (!((ident = fox_strsep(string, " ")) &&
          (host = fox_strsep(string, " ")) &&
          (server = fox_strsep(string, " ")) &&
          (nick = fox_strsep(string, " "))))
        return false;

    user = find_or_make_user(nick, ident, host);
    xfree(user->server);
    user->server = xstrdup(server);
    member = channel_get_or_add_membership(channel, user);

    *user_out = user;
    *member_out = member;

    if (!(flags = fox_strsep(string, " ")))
        return false;

    set_flags(flags, member);
    return true;
}

static void
do_parse_rpl_whoreply(char *string)
{
    const char *token;
    struct member_t *member;
    struct user_t *user;

    if (!(iparse_channel_user(&string, &user, &member) &&
          (token = fox_strsep(&string, " ")) &&
          *token++ == ':' &&
          iparse_int(&token, &user->hops)))
        return;

    xfree(member->user->gecos);
    member->user->gecos = string[0] ? xstrdup(string) : NULL;
}

void
parse_rpl_whoreply(void)
{
    char *string;
    string = xstrdup(bot.msg->params);
    do_parse_rpl_whoreply(string);
    xfree(string);
}

static void
do_parse_rpl_whospcrpl(char *string)
{
    const char *token, *account;
    struct member_t *member;
    struct user_t *user;

    if (!(iparse_channel_user(&string, &user, &member) &&
          (token = fox_strsep(&string, " ")) &&
          iparse_int(&token, &user->hops) &&
          (token = fox_strsep(&string, " ")) &&
          iparse_ulong(&token, &user->idle) &&
          (account = fox_strsep(&string, " "))))
        return;

    xfree(member->user->account);
    member->user->account = strcmp(account, "0") ? xstrdup(account) : NULL;

    xfree(member->user->gecos);
    member->user->gecos = string[0] ? xstrdup(string + 1) : NULL;
}

void
parse_rpl_whospcrpl(void)
{
    char *string;
    string = xstrdup(bot.msg->params);
    do_parse_rpl_whospcrpl(string);
    xfree(string);
}
