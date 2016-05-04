/*
 *   check_foxbot.c -- May 1 2016 9:31:02 EST
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "check_foxbot.h"
#include "check_server.h"

static void
add_testcases(Suite *s)
{
    connect_parse_setup(s);
    channel_parse_setup(s);
    memory_setup(s);
    user_parse_setup(s);
}

int
main()
{
    Suite *s = suite_create("check_foxbot");

    add_testcases(s);

    SRunner *runner = srunner_create(s);
    srunner_set_tap(runner, "-");
    srunner_run_all(runner, CK_NORMAL);

    int number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
