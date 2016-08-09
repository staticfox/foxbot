/*
 *   cap.c -- May 7 2016 00:40:26 EDT
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

#include <foxbot/cap.h>
#include <foxbot/foxbot.h>
#include <foxbot/ircd.h>
#include <foxbot/memory.h>
#include <foxbot/message.h>
#include <foxbot/utility.h>

static struct {
    const char *name;
    const unsigned value;
    const bool bot_supports;
    bool is_sticky;
} capabilities[]= {
    { "account-notify", ACCOUNTNOTIFY, true, false },
    { "account-tag", ACCOUNTTAG, false, false },
    { "away-notify", AWAYNOTIFY, false, false },
    { "cap-notify", CAPNOTIFY, false, false },
    { "chghost", CHGHOST, false, false },
    { "echo-message", ECHOMESSAGE, false, false },
    { "extended-join", EXTENDEDJOIN, false, false },
    { "invite-notify", INVITENOTIFY, false, false },
    { "multi-prefix", MULTIPREFIX, false, false },
    { "sasl", SASL, false, false },
    { "server-time", SERVERTIME, false, false },
    { "tls", TLS, false, false },
    { "userhost-in-names", USERHOSTINNAMES, false, false },
    { "staticfox.net/unit_test", FOXBOTUNITTEST, true, false }
};

static int
get_sub_command(const char *cmd)
{
    if(strcmp(cmd, "LS") == 0)
        return 1;
    if(strcmp(cmd, "ACK") == 0)
        return 2;

    return -1;
}

bool
is_sticky(unsigned int cap)
{
    for (size_t i = 0; i < CAP_OPTS; i++)
        if (capabilities[i].value == cap)
            return capabilities[i].is_sticky;

    return false;
}

static bool
set_cap(const char *cap)
{
    for (size_t i = 0; i < CAP_OPTS; i++) {
        if (!cap_supported(capabilities[i].value)) {
            if (strcmp(cap, capabilities[i].name) == 0) {
                bot.ircd->caps_supported |= capabilities[i].value;
                if (capabilities[i].bot_supports)
                    return true;
                return false;
            }
        }
    }

    return false;
}

static void
cap_ack(char *caps)
{
    char *token;

    memmove(caps, caps+1, strlen(caps));

    while ((token = fox_strsep(&caps, " ")) != NULL) {
        bool delete = false, sticky = false;
        switch(token[0]) {
            case '~':
            case '-':
                delete = true;
                memmove(token, token+1, strlen(token));
                break;
            case '=':
                sticky = true;
                memmove(token, token+1, strlen(token));
                break;
            default:
                break;
            }

        for (size_t i = 0; i < CAP_OPTS; i++) {
            if (strcmp(token, capabilities[i].name) == 0) {
                if (delete)
                    bot.ircd->caps_active &= ~(capabilities[i].value);
                else
                    bot.ircd->caps_active |= (capabilities[i].value);

                if (sticky)
                    capabilities[i].is_sticky = true;
                break;
            }
        }
    }

    if (!bot.registered)
        raw("CAP END\n");
}

static void
cap_ls(char *caps)
{
    char buf[MAX_IRC_BUF] = "";
    char *token;
    char *ptr = buf;

    memmove(caps, caps+1, strlen(caps));

    while ((token = fox_strsep(&caps, " ")) != NULL)
        if (set_cap(token))
            ptr += sprintf(ptr, "%s ", token);

    if (strlen(buf) > 0)
        ptr[strlen(ptr) - 1] = 0;

    *ptr = '\0';

    if (strlen(buf) > 0)
        raw("CAP REQ :%s\n", buf);
    else
        raw("CAP END\n");
}

void
handle_cap(void)
{
    char *token, *string, *tofree;
    int i = 0;

    tofree = string = xstrdup(bot.msg->params);

    while ((token = fox_strsep(&string, " ")) != NULL) {
        switch(i) {
        case 0:
            switch(get_sub_command(token)) {
            case 1:
                cap_ls(string);
                break;
            case 2:
                cap_ack(string);
                break;
            default:
                goto end;
            }
        default:
            goto end;
        }
    }

end:
    xfree(tofree);
}
