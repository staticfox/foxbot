/*
 *   hook.c -- May 19 2016 04:11:07 EDT
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

#include <foxbot/hook.h>
#include <foxbot/list.h>

#include "../check_foxbot.h"
#include "../check_server.h"

static int passer = 0;

void
check_func(void)
{
    passer++;
}

START_TEST(hook_check)
{
    begin_test();
    add_hook("check_hook", (hook_func)check_func);
    /* echo_test + check_hook */
    ck_assert_int_eq(hook_count(), 2);
    exec_hook("check_hook");
    ck_assert_int_eq(passer, 1);
    delete_hook((hook_func)check_func);
    ck_assert_int_eq(hook_count(), 1);
    ck_assert_int_eq(passer, 1);
    end_test();
}
END_TEST

void
hook_setup(Suite *s)
{
    TCase *tc = tcase_create("hook");

    tcase_add_checked_fixture(tc, NULL, delete_foxbot);
    tcase_add_test(tc, hook_check);

    suite_add_tcase(s, tc);
}
