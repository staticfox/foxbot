/*
 *   cap_parse.c -- May 7 2016 01:39:00 EDT
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
#include <foxbot/cap.h>
#include <foxbot/foxbot.h>
#include <foxbot/message.h>

#include "../check_foxbot.h"
#include "../check_server.h"

START_TEST(check_cap_ls)
{
    begin_test();

    ck_assert(cap_supported(EXTENDEDJOIN));
    ck_assert(!cap_supported(FOXBOTUNITTEST));

    end_test();
}
END_TEST

START_TEST(check_cap_ack)
{
    begin_test();

    write_and_wait(":ircd.staticfox.net CAP * ACK :staticfox.net/unit_test");
    ck_assert(cap_active(FOXBOTUNITTEST));
    ck_assert(!cap_active(EXTENDEDJOIN));

    end_test();
}
END_TEST

START_TEST(check_cap_modifier)
{
    begin_test();

    write_and_wait(":ircd.staticfox.net CAP * ACK :staticfox.net/unit_test");
    ck_assert(cap_active(FOXBOTUNITTEST));
    ck_assert(!cap_active(EXTENDEDJOIN));
    write_and_wait(":ircd.staticfox.net CAP * ACK :-staticfox.net/unit_test");
    ck_assert(!cap_active(FOXBOTUNITTEST));
    ck_assert(!cap_active(EXTENDEDJOIN));
    write_and_wait(":ircd.staticfox.net CAP * ACK :staticfox.net/unit_test");
    write_and_wait(":ircd.staticfox.net CAP * ACK :=staticfox.net/unit_test");
    ck_assert(cap_active(FOXBOTUNITTEST));
    ck_assert(is_sticky(FOXBOTUNITTEST));
    ck_assert(!cap_active(EXTENDEDJOIN));

    end_test();
}
END_TEST

void
cap_parse_setup(Suite *s)
{
    TCase *tc = tcase_create("cap_parse");

    tcase_add_checked_fixture(tc, NULL, NULL);
    tcase_add_test(tc, check_cap_ls);
    tcase_add_test(tc, check_cap_ack);
    tcase_add_test(tc, check_cap_modifier);

    suite_add_tcase(s, tc);
}
