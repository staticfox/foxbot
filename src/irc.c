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
#include <stdint.h>
#include <string.h>

#include "foxbot.h"
#include "message.h"
#include "memory.h"
#include "user.h"

void
handle_mode(void)
{
    /* User mode change, it *must* be us. */
    if (!(strchr(bot.ircd->supports.chan_types, bot.msg->target[0]))) {
        assert(bot.user != NULL); /* Should not be null at this point */
        if (strcmp(bot.msg->target, bot.user->nick) == 0) {
            bool adding = true;
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
        do_error("Received mode change for another user? %s", bot.msg->buffer);
        return;
    }

    /* TODO: Channel mode parser */
}

void
handle_quit(void)
{
    assert(bot.msg->from != NULL);
    delete_user_by_struct(bot.msg->from);
    bot.msg->from = NULL;
}
