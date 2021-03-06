/*
 *   channel_parse.c -- May 4 2016 00:12:16 EDT
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

START_TEST(simple_channel_check)
{
    begin_test();
    struct channel_t *chptr; /* The channel we joined */

    /* Test us joining */
    ck_assert(channel_count() == 2);
    ck_assert((chptr = find_channel("#unit_test")) != NULL);
    ck_assert(dlist_length(&chptr->users) == 1);
    ck_assert_ptr_ne(channel_get_membership(chptr, bot.user), NULL);
    ck_assert((chptr = find_channel("#test_spam")) != NULL);
    ck_assert(dlist_length(&chptr->users) == 1);
    ck_assert_ptr_ne(channel_get_membership(chptr, bot.user), NULL);
    end_test();
}
END_TEST

START_TEST(channel_part_check)
{
    begin_test();
    struct channel_t *chptr, *chptr2;
    struct user_t *uptr;
    char ibuf[MAX_IRC_BUF];

    ck_assert((chptr = find_channel("#unit_test")) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, bot.user), NULL);

    /* Introduce a new user */
    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");
    ck_assert(channel_count() == 2);
    ck_assert(dlist_length(&chptr->users) == 2);
    ck_assert((uptr = find_nick("test_user1")) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr), NULL);

    write_and_wait(":test_user1!~test@255.255.255.255 PART #unit_test :Leaving now");
    ck_assert(channel_count() == 2);
    ck_assert(dlist_length(&chptr->users) == 1);
    ck_assert(find_nick("test_user1") == NULL);

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");

    snprintf(ibuf, sizeof(ibuf), ":%s!%s@%s JOIN #unit_test2",
             bot.user->nick, bot.user->ident, bot.user->host);

    write_and_wait(ibuf);

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test2");
    ck_assert((chptr2 = find_channel("#unit_test2")) != NULL);
    write_and_wait(":test_user1!~test@255.255.255.255 PART :#unit_test2");
    ck_assert(find_nick("test_user1") != NULL);
    ck_assert(dlist_length(&chptr2->users) == 1);

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test2");
    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test2");

    wait_for_last_buf("PRIVMSG #test_spam :Attempting to rejoin test_user1 to #unit_test2.");

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_tESt");
    wait_for_last_buf("PRIVMSG #test_spam :Attempting to rejoin test_user1 to #unit_test.");

    ck_assert(dlist_length(&chptr2->users) == 2);

    snprintf(ibuf, sizeof(ibuf), ":%s!%s@%s PART #unit_test2",
             bot.user->nick, bot.user->ident, bot.user->host);

    write_and_wait(ibuf);

    snprintf(ibuf, sizeof(ibuf), ":%s!%s@%s PART #unit_test",
             bot.user->nick, bot.user->ident, bot.user->host);

    write_and_wait(ibuf);

    ck_assert(channel_count() == 1);
    ck_assert(find_channel("#unit_test") == NULL);
    end_test();
}
END_TEST

START_TEST(channel_quit_check)
{
    begin_test();

    struct channel_t *chptr, *chptr2;
    struct user_t *uptr;
    char ibuf[MAX_IRC_BUF];
    char quitbuf[MAX_IRC_BUF];

    ck_assert((chptr = find_channel("#unit_test")) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, bot.user), NULL);

    /* Introduce a new user */
    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");
    write_and_wait(":test_user2!~test@255.255.255.255 JOIN #unit_test");
    ck_assert(dlist_length(&chptr->users) == 3);
    ck_assert(user_count() == 3);
    ck_assert((uptr = find_nick("test_user2")) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr), NULL);

    write_and_wait(":test_user2!~test@255.255.255.255 QUIT :l8r1");
    ck_assert(dlist_length(&chptr->users) == 2);
    ck_assert(user_count() == 2);
    ck_assert(find_nick("test_user2") == NULL);

    snprintf(ibuf, sizeof(ibuf), ":%s!%s@%s JOIN #unit_test2",
             bot.user->nick, bot.user->ident, bot.user->host);

    write_and_wait(ibuf);
    ck_assert(channel_count() == 3);
    write_and_wait(":test_user2!~test@255.255.255.255 JOIN #unit_test");
    write_and_wait(":test_user2!~test@255.255.255.255 JOIN :#unit_test2");
    ck_assert((uptr = find_nick("test_user2")) != NULL);
    ck_assert((chptr2 = find_channel("#unit_test2")) != NULL);
    ck_assert(user_count() == 3);

    write_and_wait(":test_user2!~test@255.255.255.255 QUIT :l8r2");
    ck_assert(user_count() == 2);
    ck_assert(dlist_length(&chptr->users) == 2);
    ck_assert(dlist_length(&chptr2->users) == 1);
    ck_assert(find_nick("test_user2") == NULL);

    /* Generate a random hex value to ensure test validity */
    snprintf(quitbuf, sizeof(quitbuf), "unit test : %08x", rand());
    do_quit(quitbuf);
    yield_to_server();
    wait_for("ERROR :Closing Link: 127.0.0.1 (Quit: %s)", quitbuf);

    end_test();
}
END_TEST

START_TEST(channel_unknown_join_check)
{
    begin_test();
    write_and_wait(":ircd.staticfox.net JOIN #wat");
    ck_assert(bot.msg->from_server == true);
    ck_assert(find_channel("#wat") == NULL);
    write_and_wait(":unknown_user!~test@255.255.255.255 JOIN #unknown");
    ck_assert(bot.msg->from != bot.user);
    ck_assert(find_channel("#unknown") == NULL);
    end_test();
}
END_TEST

START_TEST(channel_unknown_part_check)
{
    begin_test();
    write_and_wait(":ircd.staticfox.net PART #wat :...");
    ck_assert(bot.msg->from_server == true);
    ck_assert(find_channel("#wat") == NULL);
    write_and_wait(":unknown_user!~test@255.255.255.255 PART #unknown :...");
    ck_assert(bot.msg->from != bot.user);
    ck_assert(find_channel("#unknown") == NULL);
    end_test();
}
END_TEST

START_TEST(channel_unknown_exists)
{
    begin_test();
    struct channel_t *chptr = xcalloc(1, sizeof(*chptr));
    chptr->name = xstrdup("shouldn't be here");
    delete_channel_s(chptr);
    ck_assert(chptr);
    ck_assert(chptr->name);
    wait_for_last_buf("PRIVMSG #test_spam :Received unknown channel struct for %p (%s)",
                      (void *)chptr, chptr->name);
    xfree(chptr->name);
    xfree(chptr);
    end_test();
}
END_TEST

START_TEST(channel_kick_user)
{
    begin_test();
    struct channel_t *chptr, *chptr2;
    struct user_t *uptr1, *uptr2;

    ck_assert((chptr = find_channel("#unit_test")) != NULL);
    ck_assert((chptr2 = find_channel("#test_spam")) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, bot.user), NULL);

    /* Introduce a new user */
    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");
    write_and_wait(":test_user2!~test@255.255.255.255 JOIN #unit_test");
    ck_assert(dlist_length(&chptr->users) == 3);
    ck_assert(user_count() == 3);
    ck_assert((uptr1 = find_nick("test_user1")) != NULL);
    ck_assert((uptr2 = find_nick("test_user2")) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr1), NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr2), NULL);

    /* test_user1 kicks test_user2 */
    write_and_wait(":test_user1!~test@255.255.255.255 KICK #unit_test test_user2 :Bye.");
    ck_assert(dlist_length(&chptr->users) == 2);
    ck_assert(user_count() == 2);
    ck_assert(find_nick("test_user2") == NULL);

    /* test_user2 comes back */
    write_and_wait(":test_user2!~test@255.255.255.255 JOIN #unit_test");
    ck_assert(dlist_length(&chptr->users) == 3);
    ck_assert(user_count() == 3);
    ck_assert((uptr2 = find_nick("test_user2")) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr2), NULL);

    /* test_user2 joins another channel we are in */
    write_and_wait(":test_user2!~test@255.255.255.255 JOIN #test_spam");
    ck_assert(dlist_length(&chptr2->users) == 2);
    ck_assert(dlist_length(&chptr->users) == 3);
    ck_assert(user_count() == 3);
    ck_assert_ptr_ne(channel_get_membership(chptr2, uptr2), NULL);

    /* test_user1 rekicks test_user2, but we should still have info
     * on test_user2 since they are in #test_spam */
    write_and_wait(":test_user1!~test@255.255.255.255 KICK #unit_test test_user2 :Bye.");
    ck_assert(dlist_length(&chptr->users) == 2);
    ck_assert(dlist_length(&chptr2->users) == 2);
    ck_assert(user_count() == 3);
    ck_assert((uptr2 = find_nick("test_user2")) != NULL);
    ck_assert(uptr2->nick != NULL);

    end_test();
}
END_TEST

START_TEST(channel_kick_me)
{
    begin_test();
    struct channel_t *chptr, *chptr2;
    struct user_t *uptr1, *uptr2;

    ck_assert((chptr = find_channel("#unit_test")) != NULL);
    ck_assert((chptr2 = find_channel("#test_spam")) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, bot.user), NULL);

    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");
    write_and_wait(":test_user2!~test@255.255.255.255 JOIN #unit_test");
    ck_assert(dlist_length(&chptr->users) == 3);
    ck_assert(user_count() == 3);
    ck_assert((uptr1 = find_nick("test_user1")) != NULL);
    ck_assert((uptr2 = find_nick("test_user2")) != NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr1), NULL);
    ck_assert_ptr_ne(channel_get_membership(chptr, uptr2), NULL);

    /* we get kicked from #unit_test */
    write_and_wait(":test_user1!~test@255.255.255.255 KICK #unit_test foxbot :Bye.");
    ck_assert((chptr = find_channel("#unit_test")) == NULL);
    ck_assert((chptr2 = find_channel("#test_spam")) != NULL);
    ck_assert(channel_count() == 1);
    ck_assert(user_count() == 1);
    ck_assert(find_nick("test_user1") == NULL);
    ck_assert(find_nick("test_user2") == NULL);

    end_test();
}
END_TEST

START_TEST(channel_unknown_kick_chan)
{
    begin_test();

    write_and_wait(":test_user1!~test@255.255.255.255 KICK #unknown test_user2 :Bye.");
    wait_for_last_buf("PRIVMSG #test_spam :Received KICK for an unknown channel #unknown");

    ck_assert(find_channel("#unknown") == NULL);
    ck_assert(find_nick("test_user1") == NULL);
    ck_assert(find_nick("test_user2") == NULL);

    end_test();
}
END_TEST

START_TEST(channel_unknown_kick_target)
{
    begin_test();
    struct channel_t *chptr;

    ck_assert((chptr = find_channel("#unit_test")) != NULL);
    write_and_wait(":test_user1!~test@255.255.255.255 JOIN #unit_test");
    ck_assert(dlist_length(&chptr->users) == 2);
    ck_assert(user_count() == 2);
    write_and_wait(":test_user1!~test@255.255.255.255 KICK #unit_test test_user2 :Bye.");
    wait_for_last_buf("PRIVMSG #test_spam :Received KICK for an unknown user test_user2");

    ck_assert(find_nick("test_user1") != NULL);
    ck_assert(find_nick("test_user2") == NULL);
    ck_assert(dlist_length(&chptr->users) == 2);
    ck_assert(user_count() == 2);

    end_test();
}
END_TEST

START_TEST(channel_who_spcrpl_ts6)
{
    begin_test();
    struct channel_t *channel;
    struct member_t *member;
    struct user_t *user;

    write_and_wait(":ircd.staticfox.net 354 foxbot #unit_test ~fox 192.168.hwo.mh ircd.staticfox.net foxbot H 0 0 0 ::3");
    write_and_wait(":ircd.staticfox.net 354 foxbot #unit_test ~kvirc64 host.two ircd.staticfox.net staticfox H*@ 0 9944 staticfox :...");
    write_and_wait(":ircd.staticfox.net 354 foxbot #unit_test ~fox 73.205.jzy.por ircd.staticfox.net windows-foxbot G+ 4 174319 0 :lol thing");
    write_and_wait(":ircd.staticfox.net 354 foxbot #unit_test ChanServ services.int services.staticfox.net ChanServ H*@ 1 0 0 :Channel Services");
    write_and_wait(":ircd.staticfox.net 315 foxbot #unit_test :End of /WHO list.");

    ck_assert_ptr_ne(user = find_nick("staticfox"), NULL);
    ck_assert_ptr_ne(channel = find_channel("#unit_test"), NULL);
    ck_assert_ptr_ne(member = channel_get_membership(channel, user), NULL);

    ck_assert(member_is_special(member) == false);
    ck_assert(member_is_owner(member) == false);
    ck_assert(member_is_admin(member) == false);
    ck_assert(member_is_op(member) == true);
    ck_assert(member_is_halfop(member) == false);
    ck_assert(member_is_voice(member) == false);
    ck_assert(member_is_userop(member) == false);

    ck_assert(user->ircop == true);
    ck_assert(user->away == false);
    ck_assert_str_eq(user->server, "ircd.staticfox.net");
    ck_assert_str_eq(user->gecos, "...");
    ck_assert_str_eq(user->account, "staticfox");
    ck_assert_int_eq(user->idle, 9944);
    ck_assert_int_eq(user->hops, 0);

    ck_assert_ptr_eq(member->user, user);

    ck_assert_ptr_ne(user = find_nick("windows-foxbot"), NULL);
    ck_assert_ptr_ne(member = channel_get_membership(channel, user), NULL);

    ck_assert(member_is_special(member) == false);
    ck_assert(member_is_owner(member) == false);
    ck_assert(member_is_admin(member) == false);
    ck_assert(member_is_op(member) == false);
    ck_assert(member_is_halfop(member) == false);
    ck_assert(member_is_voice(member) == true);
    ck_assert(member_is_userop(member) == false);

    ck_assert(user->ircop == false);
    ck_assert(user->away == true);
    ck_assert_str_eq(user->server, "ircd.staticfox.net");
    ck_assert_str_eq(user->gecos, "lol thing");
    ck_assert_ptr_eq(user->account, NULL);
    ck_assert_int_eq(user->idle, 174319);
    ck_assert_int_eq(user->hops, 4);

    ck_assert_ptr_eq(member->user, user);

    write_and_wait(":ircd.staticfox.net 354 foxbot #unknown ~fox 192.168.hwo.mh ircd.staticfox.net foxbot H 0 0 0 ::3");
    wait_for_last_buf("PRIVMSG #test_spam :Received WHO for unknown channel #unknown.");

    end_test();
}
END_TEST

START_TEST(channel_who_spcrpl_inspircd)
{
    begin_test_server(1);
    struct channel_t *channel;
    struct member_t *member;
    struct user_t *user;

    write_and_wait(":ircd.staticfox.net 352 foxbot #unit_test ~fox 192.168.hwo.mh ircd.staticfox.net foxbot H*!~@ :0 ...");
    write_and_wait(":ircd.staticfox.net 352 foxbot #unit_test ~kvirc64 host.two ircd.staticfox.net staticfox H*@& :2 ...");
    write_and_wait(":ircd.staticfox.net 352 foxbot #unit_test ~fox 73.205.jzy.por ircd.staticfox.net windows-foxbot G%+ :3 lol thing");
    write_and_wait(":ircd.staticfox.net 352 foxbot #unit_test ChanServ services.int services.staticfox.net ChanServ H*@ :0 Channel Services");
    write_and_wait(":ircd.staticfox.net 352 foxbot #unit_test :End of /WHO list.");

    ck_assert(bot.ircd->supports.whox == false);

    ck_assert_ptr_ne(user = find_nick("foxbot"), NULL);
    ck_assert_ptr_ne(channel = find_channel("#unit_test"), NULL);
    ck_assert_ptr_ne(member = channel_get_membership(channel, user), NULL);

    ck_assert(member_is_special(member) == true);
    ck_assert(member_is_owner(member) == true);
    ck_assert(member_is_admin(member) == false);
    ck_assert(member_is_op(member) == true);
    ck_assert(member_is_halfop(member) == false);
    ck_assert(member_is_voice(member) == false);
    ck_assert(member_is_userop(member) == false);

    ck_assert(user->away == false);
    ck_assert_str_eq(user->server, "ircd.staticfox.net");
    ck_assert_int_eq(user->hops, 0);
    ck_assert_str_eq(user->gecos, "...");

    ck_assert_ptr_eq(member->user, user);

    ck_assert_ptr_ne(user = find_nick("windows-foxbot"), NULL);
    ck_assert_ptr_ne(member = channel_get_membership(channel, user), NULL);

    ck_assert(member_is_special(member) == false);
    ck_assert(member_is_owner(member) == false);
    ck_assert(member_is_admin(member) == false);
    ck_assert(member_is_op(member) == false);
    ck_assert(member_is_halfop(member) == true);
    ck_assert(member_is_voice(member) == true);
    ck_assert(member_is_userop(member) == false);

    ck_assert(user->away == true);
    ck_assert_str_eq(user->server, "ircd.staticfox.net");
    ck_assert_int_eq(user->hops, 3);
    ck_assert_str_eq(user->gecos, "lol thing");

    ck_assert_ptr_eq(member->user, user);

    write_and_wait(":ircd.staticfox.net 352 foxbot #unknown ~fox 192.168.hwo.mh ircd.staticfox.net foxbot H*!~@ :0 ...");
    wait_for_last_buf("PRIVMSG #test_spam :Received WHO for unknown channel #unknown.");

    end_test();
}
END_TEST

START_TEST(channel_who_flag)
{
    begin_test_server(0);
    struct channel_t *channel;
    struct member_t *member;
    struct user_t *user;

    write_and_wait(":ircd.staticfox.net 354 foxbot #unit_test ~fox 192.168.hwo.mh ircd.staticfox.net foxbot Hd 0 0 0 ::3");
    write_and_wait(":ircd.staticfox.net 315 foxbot #unit_test :End of /WHO list.");

    ck_assert(bot.ircd->supports.whox == true);

    ck_assert_ptr_ne(user = find_nick("foxbot"), NULL);
    ck_assert_ptr_ne(channel = find_channel("#unit_test"), NULL);
    ck_assert_ptr_ne(member = channel_get_membership(channel, user), NULL);

    ck_assert_str_eq(user->flags, "d");

    end_test();
}
END_TEST

void
channel_parse_setup(Suite *s)
{
    TCase *tc = tcase_create("channel_parse");
    tcase_set_timeout(tc, 30);
    /* ^ Excessive */

    tcase_add_checked_fixture(tc, NULL, NULL);
    tcase_add_test(tc, simple_channel_check);
    tcase_add_test(tc, channel_part_check);
    tcase_add_test(tc, channel_quit_check);
    tcase_add_test(tc, channel_unknown_join_check);
    tcase_add_test(tc, channel_unknown_part_check);
    tcase_add_test(tc, channel_unknown_exists);
    tcase_add_test(tc, channel_kick_user);
    tcase_add_test(tc, channel_kick_me);
    tcase_add_test(tc, channel_unknown_kick_chan);
    tcase_add_test(tc, channel_unknown_kick_target);
    tcase_add_test(tc, channel_who_spcrpl_ts6);
    tcase_add_test(tc, channel_who_spcrpl_inspircd);
    tcase_add_test(tc, channel_who_flag);

    suite_add_tcase(s, tc);
}
