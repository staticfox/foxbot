/*
 *   cap.c -- May 7 2016 00:40:26 EST
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

static const struct {
    char *name;
    unsigned value;
    bool bot_supports;
} capabilities[]= {
    { "account-notify", ACCOUNTNOTIFY, false },
    { "account-tag", ACCOUNTTAG, false },
    { "away-notify", AWAYNOTIFY, false },
    { "cap-notify", CAPNOTIFY, false },
    { "chghost", CHGHOST, false },
    { "echo-message", ECHOMESSAGE, false },
    { "extended-join", EXTENDEDJOIN, false },
    { "invite-notify", INVITENOTIFY, false },
    { "multi-prefix", MULTIPREFIX, false },
    { "sasl", SASL, false },
    { "server-time", SERVERTIME, false },
    { "tls", TLS, false },
    { "userhost-in-names", USERHOSTINNAMES, false }
};

#define CAP_OPTS sizeof(capabilities)/sizeof(*capabilities)

static int
get_sub_command(const char *cmd)
{
    if(strcmp(cmd, "LS") == 0)
        return 1;
    if(strcmp(cmd, "ACK") == 0)
        return 2;
    if(strcmp(cmd, "REQ") == 0)
        return 3;

    return -1;
}

static bool
set_cap(const char *cap)
{
    for (size_t i = 0; i < CAP_OPTS; i++) {
        if (!(bot.ircd->caps_supported & capabilities[i].value)) {
            if (!strcmp(cap, capabilities[i].name)) {
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
        for (size_t i = 0; i < CAP_OPTS; i++) {
            if (!strcmp(token, capabilities[i].name)) {
                bot.ircd->caps_active |= capabilities[i].value;
                break;
            }
        }
    }

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
