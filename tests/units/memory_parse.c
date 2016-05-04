/*
 *   memory_parse.c -- May 4 2016 02:27:59 EST
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

#include <foxbot/channel.h>
#include <foxbot/foxbot.h>
#include <foxbot/list.h>
#include <foxbot/memory.h>
#include <foxbot/user.h>

#include "../check_foxbot.h"
#include "../check_server.h"

START_TEST(calloc_check)
{
    begin_test();

    struct user_t *user = xcalloc(1, sizeof(*user));
    ck_assert(user != NULL);
    user->nick = xstrdup("testing");
    ck_assert(user->nick != NULL);
    ck_assert(strcmp(user->nick, "testing") == 0);

    xfree(user->nick);
    xfree(user);

    end_test();
}
END_TEST

START_TEST(malloc_check)
{
    begin_test();

    struct user_t *user = xmalloc(sizeof(*user));
    memset(user, 0, sizeof(*user));

    ck_assert(user != NULL);

    user->nick = xstrdup("testing");
    ck_assert(user->nick != NULL);
    ck_assert(strcmp(user->nick, "testing") == 0);

    xfree(user->nick);
    xfree(user);

    end_test();
}
END_TEST

START_TEST(realloc_check)
{
    begin_test();

    int *test_array = xmalloc(10 * sizeof(*test_array));

    ck_assert(test_array != NULL);
    test_array = xrealloc(test_array, 1000 * sizeof(*test_array));
    assert(test_array);

    xfree(test_array);
    display_oom();

    end_test();
}
END_TEST

void
memory_setup(Suite *s)
{
    TCase *tc = tcase_create("memory_parse");

    tcase_add_checked_fixture(tc, NULL, delete_foxbot);
    tcase_add_test(tc, calloc_check);
    tcase_add_test(tc, malloc_check);
    tcase_add_test(tc, realloc_check);

    suite_add_tcase(s, tc);
}
