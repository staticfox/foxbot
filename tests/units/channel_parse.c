/*
 *   channel_parse.c -- May 4 2016 00:12:16 EST
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
#include <foxbot/message.h>
#include <foxbot/user.h>

#include "../check_foxbot.h"
#include "../check_server.h"

START_TEST(simple_channel_check)
{
    begin_test();
    struct channel_t *chptr; /* The channel we joined */
    struct user_t *uptr; /* me */

    /* Test us joining */
    ck_assert(channel_count() == 1);
    ck_assert((chptr = find_channel("#unit_test")) != NULL);
    ck_assert(dlist_length(chptr->users) == 1);
    ck_assert((uptr = channel_get_user(chptr, bot.user)) != NULL);
    end_test();
}
END_TEST

START_TEST(channel_part_check)
{
    begin_test();
    struct channel_t *chptr, *chptr2;
    struct user_t *uptr, *uptr2;
    char ibuf[MAX_IRC_BUF];

    ck_assert((chptr = find_channel("#unit_test")) != NULL);
    ck_assert((uptr = channel_get_user(chptr, bot.user)) != NULL);

    /* Introduce a new user */
    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");
    ck_assert(channel_count() == 1);
    ck_assert(dlist_length(chptr->users) == 2);
    ck_assert((uptr2 = get_user_by_nick("test_user1")) != NULL);
    ck_assert(channel_get_user(chptr, uptr2) != NULL);

    write_and_wait(":test_user1!~test@255.255.255.255 PART #unit_test");
    ck_assert(channel_count() == 1);
    ck_assert(dlist_length(chptr->users) == 1);
    ck_assert(get_user_by_nick("test_user1") == NULL);

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");

    snprintf(ibuf, sizeof(ibuf), ":%s!%s@%s JOIN #unit_test2",
        bot.user->nick, bot.user->ident, bot.user->host);

    write_and_wait(ibuf);

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test2");
    ck_assert((chptr2 = find_channel("#unit_test2")) != NULL);
    write_and_wait(":test_user1!~test@255.255.255.255 PART #unit_test2");
    ck_assert(get_user_by_nick("test_user1") != NULL);
    ck_assert(dlist_length(chptr2->users) == 1);

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test2");

    snprintf(ibuf, sizeof(ibuf), ":%s!%s@%s PART #unit_test2",
        bot.user->nick, bot.user->ident, bot.user->host);

    write_and_wait(ibuf);

    snprintf(ibuf, sizeof(ibuf), ":%s!%s@%s PART #unit_test",
        bot.user->nick, bot.user->ident, bot.user->host);

    write_and_wait(ibuf);

    ck_assert(channel_count() == 0);
    ck_assert(find_channel("#unit_test") == NULL);
    end_test();
}
END_TEST

START_TEST(channel_quit_check)
{
    begin_test();

    struct channel_t *chptr, *chptr2;
    struct user_t *uptr, *uptr2;
    char ibuf[MAX_IRC_BUF];

    ck_assert((chptr = find_channel("#unit_test")) != NULL);
    ck_assert((uptr = channel_get_user(chptr, bot.user)) != NULL);

    /* Introduce a new user */
    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");
    write_and_wait(":test_user2!~test@255.255.255.255 JOIN #unit_test");
    ck_assert(dlist_length(chptr->users) == 3);
    ck_assert(user_count() == 3);
    ck_assert((uptr2 = get_user_by_nick("test_user2")) != NULL);
    ck_assert(channel_get_user(chptr, uptr2) != NULL);

    write_and_wait(":test_user2!~test@255.255.255.255 QUIT :l8r");
    ck_assert(dlist_length(chptr->users) == 2);
    ck_assert(user_count() == 2);
    ck_assert(get_user_by_nick("test_user2") == NULL);

    snprintf(ibuf, sizeof(ibuf), ":%s!%s@%s JOIN #unit_test2",
        bot.user->nick, bot.user->ident, bot.user->host);

    write_and_wait(ibuf);
    ck_assert(channel_count() == 2);
    write_and_wait(":test_user2!~test@255.255.255.255 JOIN #unit_test");
    write_and_wait(":test_user2!~test@255.255.255.255 JOIN #unit_test2");
    ck_assert((uptr2 = get_user_by_nick("test_user2")) != NULL);
    ck_assert((chptr2 = find_channel("#unit_test2")) != NULL);
    ck_assert(user_count() == 3);

    write_and_wait(":test_user2!~test@255.255.255.255 QUIT :l8r");
    ck_assert(user_count() == 2);
    ck_assert(dlist_length(chptr->users) == 2);
    ck_assert(dlist_length(chptr2->users) == 1);
    ck_assert(get_user_by_nick("test_user2") == NULL);

    end_test();
}
END_TEST

void
channel_parse_setup(Suite *s)
{
    TCase *tc = tcase_create("channel_parse");

    tcase_add_checked_fixture(tc, NULL, delete_foxbot);
    tcase_add_test(tc, simple_channel_check);
    tcase_add_test(tc, channel_part_check);
    tcase_add_test(tc, channel_quit_check);

    suite_add_tcase(s, tc);
}
