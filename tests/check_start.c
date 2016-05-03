/*
 *   start.c -- May 1 2016 19:44:55 EST
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
#include <pthread.h>
#include <stdio.h>

#include <foxbot/config.h>
#include <foxbot/foxbot.h>
#include <foxbot/message.h>
#include <foxbot/socket.h>

#include "check_foxbot.h"
#include "check_server.h"

extern int main_foxbot(int argc, char **argv);
extern void io();

pthread_t tid[3];

void
delete_foxbot(void)
{
    foxbot_quit();
}

void
new_foxbot(void)
{
    char *args[] = {
        PREFIX "/bin/foxbot",
        "-t"
    };

    main_foxbot(sizeof(args) / sizeof(*args), args);
}

void
new_testserver(void)
{
    int fd;
    int err;

    if((fd = setup_test_server()) < 0) {
        fprintf(stderr, "Unable to start socket server.\n");
        return;
    }

    err = pthread_create(&(tid[0]), NULL, start_listener, NULL);

    if(err != 0) {
        fprintf(stderr, "Thread error: %s\n", strerror(errno));
        return;
    }
}

void
begin_test(void)
{
    new_testserver();
    new_foxbot();
}

void
end_test(void)
{
    delete_foxbot();
    tests_done = 1;
    shutdown_test_server();
}

void
wait_for(const char *data)
{
    for (;;) {
        if (bot.msg->command && (strcmp(bot.msg->command, data) == 0))
            break;
        io();
    }
}
