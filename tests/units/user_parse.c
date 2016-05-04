/*
 *   user_parse.c -- May 4 2016 05:05:39 EST
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

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include <foxbot/foxbot.h>
#include <foxbot/message.h>
#include <foxbot/user.h>

#include "../check_foxbot.h"
#include "../check_server.h"

START_TEST(user_mode_check)
{
    begin_test();
    char buf[MAX_IRC_BUF];

    snprintf(buf, sizeof(buf), ":%s MODE %s :-i", bot.user->nick, bot.user->nick);

    write_and_wait(buf);
    ck_assert(!(bot.modes[(uint8_t)'i']));

    snprintf(buf, sizeof(buf), ":%s MODE %s :+R", bot.user->nick, bot.user->nick);

    write_and_wait(buf);
    ck_assert(bot.modes[(uint8_t)'R']);

    write_and_wait(":UNKNOWN MODE UNKNOWN :+i");

    for (;;) {
        if (strcmp(last_buffer, "PRIVMSG #unit_test :Received mode change for another user? :UNKNOWN MODE UNKNOWN :+i") == 0)
            break;
    }

    ck_assert(!(bot.modes[(uint8_t)'i']));

    end_test();
}
END_TEST

void
user_parse_setup(Suite *s)
{
    TCase *tc = tcase_create("user_parse");

    tcase_add_checked_fixture(tc, NULL, delete_foxbot);
    tcase_add_test(tc, user_mode_check);

    suite_add_tcase(s, tc);
}
