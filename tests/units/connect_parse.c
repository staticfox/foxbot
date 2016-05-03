/*
 *   connect_parse.c -- May 1 2016 19:42:39 EST
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

#include <stdint.h>
#include <stdio.h>

#include <foxbot/foxbot.h>
#include <foxbot/list.h>
#include <foxbot/user.h>

#include "../check_foxbot.h"
#include "../check_server.h"

START_TEST(connect)
{
    begin_test();

    ck_assert(strcmp(bot.user->nick, "foxbot") == 0);
    ck_assert(strcmp(bot.user->ident, "~fox") == 0);
    ck_assert(strcmp(bot.user->host, "127.0.0.1") == 0);
    ck_assert(bot.modes[(uint8_t)'i']);

    end_test();
}
END_TEST

START_TEST(ircd_support)
{
    begin_test();

    ck_assert(bot.ircd->supports.excepts);
    ck_assert(bot.ircd->supports.invex);
    ck_assert(bot.ircd->supports.knock);
    ck_assert(bot.ircd->supports.whox);
    ck_assert(strcmp(bot.ircd->network, "StaticFox") == 0);
    ck_assert(strcmp(bot.ircd->supports.chan_types, "&#") == 0);
    ck_assert(bot.ircd->nick_length == 30);
    ck_assert(bot.ircd->channel_length == 50);
    ck_assert(bot.ircd->topic_length == 390);
    ck_assert(strcmp(bot.ircd->supports.chanop_modes, "ov") == 0);
    ck_assert(strcmp(bot.ircd->supports.prefix, "@+") == 0);

    end_test();
}
END_TEST

void
connect_parse_setup(Suite *s)
{
    TCase *tc = tcase_create("connect_parse");

    tcase_add_checked_fixture(tc, NULL, delete_foxbot);
    tcase_add_test(tc, connect);
    tcase_add_test(tc, ircd_support);

    suite_add_tcase(s, tc);
}
