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
#include <foxbot/socket.h>
#include <foxbot/user.h>

#include "../check_foxbot.h"
#include "../check_server.h"

START_TEST(connect_check)
{
    begin_test();

    ck_assert(strcmp(bot.user->nick, "foxbot") == 0);
    ck_assert(strcmp(bot.user->ident, "~fox") == 0);
    ck_assert(strcmp(bot.user->host, "127.0.0.1") == 0);
    ck_assert(bot.modes[(uint8_t)'i']);

    end_test();
}
END_TEST

START_TEST(ircd_support_check)
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

    send_broken_uint_value();
    wait_for(":ircd.staticfox.net 005 %s NICKLEN=-30 :are supported by this server", bot.user->nick);

    ck_assert(bot.ircd->nick_length == 30);

    end_test();
}
END_TEST

START_TEST(ircd_ping_pong)
{
    begin_test();

    write_and_wait("PING :ircd.staticfox.net");

    wait_for_command(PING);

    end_test();
}
END_TEST

START_TEST(ircd_unknown_cmd)
{
    begin_test();
    sockwrite("ASDF\r\n");
    wait_for_numeric(421);
    end_test();
}
END_TEST

START_TEST(ircd_invalid_cmd)
{
    begin_test();
    write_and_wait("ASDF ASDF");
    ck_assert(bot.msg->from == NULL);
    ck_assert(bot.msg->command == NULL);
    ck_assert(bot.msg->is_invalid);

    write_and_wait("ASDF ASDF ASDF ASDF");
    ck_assert(bot.msg->is_invalid);

    end_test();
}
END_TEST

START_TEST(ircd_error)
{
    begin_test();
    write_and_wait("ERROR :Closing Link: 127.0.0.1 (Quit: tests!)");
    ck_assert(quitting == 1);
    end_test();
}
END_TEST

void
connect_parse_setup(Suite *s)
{
    TCase *tc = tcase_create("connect_parse");

    tcase_add_checked_fixture(tc, NULL, delete_foxbot);
    tcase_add_test(tc, connect_check);
    tcase_add_test(tc, ircd_support_check);
    tcase_add_test(tc, ircd_ping_pong);
    tcase_add_test(tc, ircd_unknown_cmd);
    tcase_add_test(tc, ircd_invalid_cmd);
    tcase_add_test(tc, ircd_error);

    suite_add_tcase(s, tc);
}
