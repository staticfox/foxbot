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

/* Test plans here soon^tm */

#include <stdio.h>

#include "check_foxbot.h"
#include "check_server.h"

int
main(/*int argc, char **argv*/)
{
    int fd;
    if((fd = setup_test_server()) < 0) {
        fprintf(stderr, "Unable to start socket server.\n");
        return 1;
    }

    /* Must be started in a thread/background process */
    /* start_listener(fd); */

    return 0;
}
