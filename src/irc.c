/*
 *   irc.c -- April 29 2016 10:12:47 EST
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "channel.h"
#include "foxbot.h"
#include "message.h"
#include "memory.h"
#include "user.h"

void
handle_mode(void)
{
    /* User mode change, it *must* be us. */
    if (!(strchr(bot.ircd->supports.chan_types, bot.msg->target[0]))) {
        bool adding = true;

        assert(bot.user != NULL); /* Should not be null at this point */

        if (strcmp(bot.msg->target, bot.user->nick) != 0) {
            do_error("Received mode change for another user? %s", bot.msg->buffer);
            return;
        }

        for (size_t ii = 0; bot.msg->params[ii] != '\0'; ii++) {
            switch (bot.msg->params[ii]) {
            case ':':
                continue;
            case '+':
                adding = true;
                continue;
            case '-':
                adding = false;
                continue;
            default:
                assert(isalnum(bot.msg->params[ii]));
                bot.modes[(uint8_t)bot.msg->params[ii]] = adding;
                continue;
            }
        }

    return;
    }

    /* TODO: Channel mode parser */
}

void
handle_join(void)
{
    /* eh? */
    if (bot.msg->from_server == true)
        return;

    /* We know their full n!u@h now, incase we just got them from
     * /NAMES */
    if (bot.msg->from->ident == NULL || bot.msg->from->host == NULL)
        set_uh(bot.msg->from, bot.msg->source);

    struct channel_t *channel = NULL;
    channel = find_channel(bot.msg->target);

    if (channel == NULL && bot.msg->from != bot.user) {
        do_error("Received JOIN for an unknown channel: %s", bot.msg->buffer);
        return;
    }

    /* Must be us */
    if (channel == NULL)
        channel = create_channel(bot.msg->target);

    add_user_to_channel(channel, bot.msg->from);
}

void
handle_part(void)
{
    /* eh? */
    if (bot.msg->from_server == true)
        return;

    struct channel_t *channel = NULL;
    channel = find_channel(bot.msg->target);

    if (channel == NULL) {
        do_error("Received PART for an unknown channel %s", bot.msg->target);
        return;
    }

    /* We left, clear out all data */
    if (bot.msg->from == bot.user) {
        delete_channel_s(channel);
        return;
    }

    channel_remove_user(channel, bot.msg->from);
}

void
handle_quit(void)
{
    assert(bot.msg->from != NULL);

    /* SHOULD delete the user's struct on channel_remove_user */
    channel_quit_user(bot.msg->from);
}
