/*
 *   check_server.c -- May 1 2016 10:58:00 EST
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
#define _POSIX_C_SOURCE 201112L

#include <time.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <foxbot/memory.h>
#include <foxbot/message.h>
#include <foxbot/foxbot.h>

#include "check_server.h"

int tests_done = 0;
int client_sock_fd = 0;
int sockfd = 0;

enum check_commands {
    INVALID,
    NICK,
    USER
};

bool got_nick = false, got_user = false, connected = false;
char *check_nick, *check_user;

void
do_burst(void)
{
    fox_write(client_sock_fd, ":ircd.staticfox.net NOTICE * :*** Ident disabled, not checking ident\r\n");
    fox_write(client_sock_fd, ":ircd.staticfox.net NOTICE * :*** Looking up your hostname...\r\n");
    fox_write(client_sock_fd, ":ircd.staticfox.net 001 %s :Welcome to the StaticFox Internet Relay Chat Network %s\r\n", check_nick, check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 002 %s :Your host is ircd.staticfox.net[ircd.staticfox.net/9990], running version generic-ircd-1\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 003 %s :This server was created Sat Apr 30 2016 at 21:34:09 EDT\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 004 %s ircd.staticfox.net generic-ircd-1 DQRSZagiloswz CFILPQbcefgijklmnopqrstvz bkloveqjfI\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 005 %s CPRIVMSG CNOTICE MONITOR=100 WHOX ETRACE SAFELIST ELIST=CTU KNOCK FNC CHANTYPES=&# EXCEPTS INVEX :are supported by this server\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 005 %s CHANMODES=eIbq,k,flj,CFLPQcgimnprstz CHANLIMIT=&#:15 PREFIX=(ov)@+ MAXLIST=bqeI:100 MODES=4 NETWORK=StaticFox STATUSMSG=@+ CALLERID=g CASEMAPPING=rfc1459 NICKLEN=30 MAXNICKLEN=31 CHANNELLEN=50 :are supported by this server\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 005 %s TOPICLEN=390 DEAF=D TARGMAX=NAMES:1,LIST:1,KICK:1,WHOIS:1,PRIVMSG:4,NOTICE:4,ACCEPT:,MONITOR: CLIENTVER=3.0 :are supported by this server\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 251 %s :There are 0 users and 18 invisible on 2 servers\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 252 %s 1 :IRC Operators online\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 254 %s 4 :channels formed\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 255 %s :I have 3 clients and 1 servers\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 265 %s 3 4 :Current local users 3, max 4\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 266 %s 18 19 :Current global users 18, max 19\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 250 %s :Highest connection count: 5 (4 clients) (30 connections received)\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 375 %s :- ircd.staticfox.net Message of the Day -\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 372 %s :- Not an important MOTD\r\n", check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 376 %s :End of /MOTD command.\r\n", check_nick);
    fox_write(client_sock_fd, ":%s MODE %s :+i\r\n", check_nick, check_nick);

    /* Needed to give the bot an idea of what it's n!u@h is */
    fox_write(client_sock_fd, ":%s!~%s@127.0.0.1 JOIN #unit_test\r\n", check_nick, check_user);
    fox_write(client_sock_fd, ":ircd.staticfox.net 353 %s = #unit_test :%s\r\n", check_nick, check_nick);
    fox_write(client_sock_fd, ":ircd.staticfox.net 366 %s #unit_test :End of /NAMES list.\r\n", check_nick);

    /* Used as a reference point to know when the spam has stopped */
    fox_write(client_sock_fd, ":ircd.staticfox.net FOXBOT * :Not a real command :)\r\n");
}

/* Should we even bother parsing? */
static void
parse_buffer(const char *buf)
{
    enum check_commands cmd = INVALID;
    unsigned int i = 0, ii;
    int params = 1;
    char *token, *string, *tofree;

    for (ii = 0; buf[ii] != '\0'; ii++)
        if (buf[ii] == ' ')
            params++;

    tofree = string = xstrdup(buf);

    while (((token = fox_strsep(&string, " ")) != NULL) && i < 3) {
        switch(i) {
        case 0:
            if (strcmp(token, "NICK") == 0)
                cmd = NICK;
            else if (strcmp(token, "USER") == 0)
                cmd = USER;
            break;
        case 1:
            if (cmd == NICK) {
                check_nick = xstrdup(token);
                got_nick = true;
                goto end;
            } else if (cmd == USER) {
                check_user = xstrdup(token);
                got_user = true;
                goto end;
            }
            break;
        }
        i++;
    }

end:
    xfree(tofree);

    if (got_nick && got_user && !connected) {
        connected = true;
        do_burst();
    }

}

void
fox_write(int fd, char *line, ...)
{
    char buf[MAX_IRC_BUF] = {0};
    ssize_t writeval;
    int n;

    va_list ap;
    va_start(ap, line);
    vsnprintf(buf, MAX_IRC_BUF, line, ap);
    va_end(ap);

    writeval = strlen(buf);

    /* This is mostly for burst spam control */
    struct timespec tim;
    tim.tv_sec  = 0;
    tim.tv_nsec = 250000L;
    nanosleep(&tim , NULL);

    n = write(fd, buf, writeval);

    if (n < 0) {
       fprintf(stderr, "[Server] Write error: %s\n", strerror(errno));
       exit(EXIT_FAILURE);
    }
}

void
fox_read(int fd)
{
    static char inbuf[MAX_IRC_BUF];
    static size_t buf_used;
    size_t buf_remain = sizeof(inbuf) - buf_used;

    if (buf_remain == 0) {
        fprintf(stderr, "[Server] Line exceeded buffer length\n");
        tests_done = 1;
        return;
    }

    ssize_t rv = recv(fd, inbuf + buf_used, buf_remain, 0);

    if (rv == 0) {
        fprintf(stderr, "[Server] Remote host closed the connection\n");
        tests_done = 1;
        return;
    }

    if (rv < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return;

    if (rv < 0) {
        fprintf(stderr, "[Server] Read error: %s\n", strerror(errno));
        tests_done = 1;
        return;
    }

    buf_used += rv;

    char *line_start = inbuf;
    char *line_end;
    while ((line_end = memchr(line_start, '\n', buf_used))) {
        *line_end = '\0';

        if (strlen(line_start) > 0 && line_end[-1] == '\r')
            line_end[-1] = '\0';

        assert(strlen(line_start) <= MAX_IRC_BUF);
        parse_buffer(line_start);
        ++line_end;
        buf_used -= line_end - line_start;
        line_start = line_end;
    }

    memmove(inbuf, line_start, buf_used);
}

void *
start_listener(void *unused)
{
    (void)unused;

    struct sockaddr_in cli_addr;
    socklen_t clilen;

    memset(&cli_addr, 0, sizeof(cli_addr));

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    client_sock_fd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if (client_sock_fd < 0) {
       fprintf(stderr, "[Server] Accept error: %s\n", strerror(errno));
       exit(EXIT_FAILURE);
    }

    while(!tests_done) {
        fox_read(client_sock_fd);
    }

    close(client_sock_fd);

    return NULL;
}

void
shutdown_test_server(void)
{
    close(sockfd);
}

int
setup_test_server(void)
{
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
       fprintf(stderr, "[Server] Socket error: %s", strerror(errno));
       return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(43255);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "[Server] Bind error: %s\n", strerror(errno));
        return -1;
    }

    return sockfd;
}
