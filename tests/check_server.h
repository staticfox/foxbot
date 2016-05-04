/*
 *   check_server.h -- May 1 2016 9:39:18 EST
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

#include <stdbool.h>

int setup_test_server(void);
void fox_write(char *line, ...);
void fox_read(int fd);
void * start_listener(void *unused);
void shutdown_test_server(void);
void delete_foxbot(void);
void write_and_wait(char *data);
void send_broken_uint_value(void);

extern int tests_done;
extern int client_sock_fd;
extern int sockfd;
bool got_nick, got_user;
char *check_nick, *check_user;
