/*
 *   admin_parse.c -- May 20 2016 07:55:46 EDT
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

#include <foxbot/admin.h>
#include <foxbot/channel.h>
#include <foxbot/foxbot.h>
#include <foxbot/message.h>
#include <foxbot/user.h>

#include "../check_foxbot.h"
#include "../check_server.h"

START_TEST(check_admin_nickserv)
{
    begin_test();
    struct user_t *uptr;

    ck_assert_ptr_ne(find_channel("#unit_test"), NULL);

    write_and_wait(":test_user1!~test@127.0.0.10 JOIN #unit_test");
    ck_assert((uptr = find_nick("test_user1")) != NULL);
    ck_assert_ptr_eq(uptr->account, NULL);
    write_and_wait(":test_user1!~test@255.255.255.255 ACCOUNT god");
    ck_assert_str_eq(uptr->account, "god");
    ck_assert_int_eq(find_admin_access(uptr), 785);
    ck_assert_int_ne(find_admin_access(uptr), 453);

    end_test();

}
END_TEST

START_TEST(check_admin_host)
{
    begin_test();
    struct user_t *uptr;

    ck_assert_ptr_ne(find_channel("#unit_test"), NULL);

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");
    ck_assert((uptr = find_nick("test_user1")) != NULL);
    ck_assert_int_eq(find_admin_access(uptr), 453);
    ck_assert_int_ne(find_admin_access(uptr), 785);

    end_test();
}
END_TEST

START_TEST(check_admin_noaccess)
{
    begin_test();
    struct user_t *uptr;

    ck_assert_ptr_ne(find_channel("#unit_test"), NULL);

    write_and_wait(":test_user1!~test@255.255.255.0 JOIN #unit_test");
    ck_assert((uptr = find_nick("test_user1")) != NULL);
    ck_assert_int_eq(find_admin_access(uptr), 0);

    end_test();
}
END_TEST

START_TEST(check_admin_count)
{
    begin_test();

    ck_assert_int_eq(admin_count(), 2);

    end_test();
}
END_TEST

void
admin_setup(Suite *s)
{
    TCase *tc = tcase_create("admin_parse");

    tcase_add_checked_fixture(tc, NULL, NULL);
    tcase_add_test(tc, check_admin_nickserv);
    tcase_add_test(tc, check_admin_host);
    tcase_add_test(tc, check_admin_noaccess);
    tcase_add_test(tc, check_admin_count);

    suite_add_tcase(s, tc);
}
