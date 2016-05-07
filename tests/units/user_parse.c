/*
 *   user_parse.c -- May 4 2016 05:05:39 EDT
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

#include <foxbot/channel.h>
#include <foxbot/conf.h>
#include <foxbot/foxbot.h>
#include <foxbot/list.h>
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

    wait_for_last_buf("PRIVMSG %s :Received mode change for another user? :UNKNOWN MODE UNKNOWN :+i",
                      botconfig.debug_channel);

    ck_assert(!(bot.modes[(uint8_t)'i']));

    end_test();
}
END_TEST

START_TEST(user_nick)
{
    begin_test();
    struct channel_t *chptr;
    struct user_t *uptr;

    ck_assert((chptr = find_channel("#unit_test")) != NULL);

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");
    ck_assert(dlist_length(&chptr->users) == 2);
    ck_assert(user_count() == 2);
    ck_assert((uptr = find_nick("test_user1")) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr), NULL);

    write_and_wait(":test_user1!~test@255.255.255.255 NICK :newnick1");
    ck_assert(find_nick("newnick1") == uptr);
    ck_assert(dlist_length(&chptr->users) == 2);
    ck_assert(user_count() == 2);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr), NULL);

    end_test();
}
END_TEST

START_TEST(user_nick_me)
{
    begin_test();
    struct channel_t *chptr;
    struct user_t *uptr;
    char buf[MAX_IRC_BUF];

    ck_assert((chptr = find_channel("#unit_test")) != NULL);

    ck_assert(dlist_length(&chptr->users) == 1);
    ck_assert(user_count() == 1);
    ck_assert((uptr = find_nick(bot.user->nick)) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr), NULL);

    snprintf(buf, sizeof(buf), ":%s!%s@127.0.0.1 NICK :newme",
             bot.user->nick, bot.user->ident);

    write_and_wait(buf);
    ck_assert(find_nick("newme") == uptr);
    ck_assert(dlist_length(&chptr->users) == 1);
    ck_assert(user_count() == 1);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr), NULL);
    ck_assert(strcmp(bot.user->nick, "newme") == 0);

    end_test();
}
END_TEST

START_TEST(user_nick_server)
{
    begin_test();
    char buf[MAX_IRC_BUF];

    snprintf(buf, sizeof(buf), ":%s NICK :newserver", bot.ircd->name);
    write_and_wait(buf);

    /* baka */
    ck_assert(strcmp(bot.ircd->name, "newserver") != 0);

    end_test();
}
END_TEST

void
user_parse_setup(Suite *s)
{
    TCase *tc = tcase_create("user_parse");

    tcase_add_checked_fixture(tc, NULL, delete_foxbot);
    tcase_add_test(tc, user_mode_check);
    tcase_add_test(tc, user_nick);
    tcase_add_test(tc, user_nick_me);
    tcase_add_test(tc, user_nick_server);

    suite_add_tcase(s, tc);
}
