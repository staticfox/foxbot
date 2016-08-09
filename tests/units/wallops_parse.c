/*
 *   wallops_parse.c -- Aug 8 2016 22:07:42 EDT
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
#include <foxbot/foxbot.h>
#include <foxbot/message.h>
#include <foxbot/user.h>

#include "../check_foxbot.h"
#include "../check_server.h"

START_TEST(check_wallops)
{
    begin_test();

    write_and_wait(":staticfox!fox@some.host WALLOPS :OPERWALL - Test");
    ck_assert_int_eq(bot.msg->ctype, WALLOPS);
    ck_assert_str_eq(bot.msg->source, "staticfox!fox@some.host");
    ck_assert_str_eq(bot.msg->command, "WALLOPS");
    ck_assert_str_eq(bot.msg->params, ":OPERWALL - Test");

    end_test();
}
END_TEST

void
wallops_parse_setup(Suite *s)
{
    TCase *tc = tcase_create("wallops_parse");

    tcase_add_checked_fixture(tc, NULL, delete_foxbot);
    tcase_add_test(tc, check_wallops);

    suite_add_tcase(s, tc);
}
