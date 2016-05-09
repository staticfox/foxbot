/*
 *   ircv3_parse.c -- May 8 2016 20:44:54 EDT
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
#include <stdlib.h>

#include <foxbot/channel.h>
#include <foxbot/conf.h>
#include <foxbot/foxbot.h>
#include <foxbot/list.h>
#include <foxbot/memory.h>
#include <foxbot/message.h>
#include <foxbot/user.h>

#include "../check_foxbot.h"
#include "../check_server.h"

START_TEST(account_notify_check)
{
    begin_test();
    struct user_t *uptr;

    ck_assert(botconfig.channel && botconfig.debug_channel);
    ck_assert_ptr_ne(find_channel(botconfig.channel), NULL);

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");
    ck_assert((uptr = find_nick("test_user1")) != NULL);
    ck_assert_ptr_eq(uptr->account, NULL);
    write_and_wait(":test_user1!~test@255.255.255.255 ACCOUNT testaccount");
    ck_assert_str_eq(uptr->account, "testaccount");
    write_and_wait(":test_user1!~test@255.255.255.255 ACCOUNT newaccount");
    ck_assert_str_eq(uptr->account, "newaccount");
    write_and_wait(":test_user1!~test@255.255.255.255 ACCOUNT *");
    ck_assert_ptr_eq(uptr->account, NULL);

    end_test();
}
END_TEST

void
ircv3_parse_setup(Suite *s)
{
    TCase *tc = tcase_create("ircv3_parse");

    tcase_add_checked_fixture(tc, NULL, delete_foxbot);
    tcase_add_test(tc, account_notify_check);

    suite_add_tcase(s, tc);
}
