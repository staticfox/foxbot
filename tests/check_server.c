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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
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
int client_sock_fd;
int sockfd;

enum check_commands {
    NICK,
    USER
};

bool got_nick = false, got_user = false;
char *check_nick, *check_user;

static void
do_burst(void)
{
    fox_write(client_sock_fd, ":hades.arpa NOTICE * :*** Ident disabled, not checking ident\r\n");
    fox_write(client_sock_fd, ":hades.arpa NOTICE * :*** Looking up your hostname...\r\n");
    fox_write(client_sock_fd, ":hades.arpa 001 %s :Welcome to the StaticFox Internet Relay Chat Network %s\r\n", check_nick, check_nick);
    fox_write(client_sock_fd, ":hades.arpa 002 %s :Your host is hades.arpa[hades.arpa/9990], running version charybdis-4-beta1\r\n");
    fox_write(client_sock_fd, ":hades.arpa 003 %s :This server was created Sat Apr 30 2016 at 21:34:09 EDT\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 004 %s hades.arpa charybdis-4-beta1 DQRSZagiloswz CFILPQbcefgijklmnopqrstvz bkloveqjfI\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 005 %s CPRIVMSG CNOTICE MONITOR=100 WHOX ETRACE SAFELIST ELIST=CTU KNOCK FNC CHANTYPES=&# EXCEPTS INVEX :are supported by this server\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 005 %s CHANMODES=eIbq,k,flj,CFLPQcgimnprstz CHANLIMIT=&#:15 PREFIX=(ov)@+ MAXLIST=bqeI:100 MODES=4 NETWORK=StaticFox STATUSMSG=@+ CALLERID=g CASEMAPPING=rfc1459 NICKLEN=30 MAXNICKLEN=31 CHANNELLEN=50 :are supported by this server\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 005 %s TOPICLEN=390 DEAF=D TARGMAX=NAMES:1,LIST:1,KICK:1,WHOIS:1,PRIVMSG:4,NOTICE:4,ACCEPT:,MONITOR: CLIENTVER=3.0 :are supported by this server\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 251 %s :There are 0 users and 18 invisible on 2 servers\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 252 %s 1 :IRC Operators online\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 254 %s 4 :channels formed\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 255 %s :I have 3 clients and 1 servers\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 265 %s 3 4 :Current local users 3, max 4\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 266 %s 18 19 :Current global users 18, max 19\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 250 %s :Highest connection count: 5 (4 clients) (30 connections received)\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 375 %s :- hades.arpa Message of the Day -\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 372 %s :- This is charybdis MOTD you might replace it, but if not your friends will\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 372 %s :- laugh at you.\r\n", check_nick);
    fox_write(client_sock_fd, ":hades.arpa 376 %s :End of /MOTD command.\r\n", check_nick);
    fox_write(client_sock_fd, ":%s MODE %s :+i\r\n", check_nick, check_nick);
    fox_write(client_sock_fd, ":%s!~%s@127.0.0.1 JOIN #unit_test\r\n", check_nick, check_user);
    fox_write(client_sock_fd, ":hades.arpa 353 %s = #chat :%s staticfox xofcitats @ChanServ\r\n", check_nick, check_nick);
    fox_write(client_sock_fd, ":hades.arpa 366 %s #chat :End of /NAMES list.\r\n", check_nick);
}

/* Should we even bother parsing? */
static void
parse_buffer(const char *buf)
{
    enum check_commands cmd = 0;
    unsigned int i = 0, ii;
    int params = 1;
    char *token, *string, *tofree;

    fprintf(stderr, "in parse_buffer()\n");

    for (ii = 0; buf[ii] != '\0'; ii++)
        if (buf[ii] == ' ')
            params++;

    tofree = string = xstrdup(buf);

    while (((token = fox_strsep(&string, " ")) != NULL) && i < 3) {
        switch(i) {
        case 0:
            if (strcmp(token, "NICK")) {
                cmd = NICK;
                got_nick = true;
                goto end;
            }
            else if (strcmp(token, "USER")) {
                cmd = USER;
                got_user = true;
                goto end;
            }
            break;
        case 1:
            if (cmd == NICK)
                check_nick = xstrdup(token);
            else if (cmd == USER)
                check_user = xstrdup(token);
            break;
        i++;
        }
    }

end:
    xfree(tofree);

    if (got_nick && got_user)
        do_burst();

}

void
fox_write(int fd, char *line, ...)
{
    char buf[MAX_IO_BUF] = {0};
    ssize_t writeval;
    int n;

    va_list ap;
    va_start(ap, line);
    vsnprintf(buf, MAX_IO_BUF, line, ap);
    va_end(ap);

    writeval = strlen(buf);

    n = write(fd, buf, writeval);

    if (n < 0) {
       fprintf(stderr, "[Server] Write error: %s\n", strerror(errno));
       exit(EXIT_FAILURE);
    }
}

/* Basically copy pasted from socket.c. To understand
 * the implementation better, check the comments in
 * the main socket.c file as well as the git revision
 * history. This is also pretty un-necessary, considering
 * we are physically controlling what gets sent so why we
 * need to have a complicated read system instead of just
 * piping it out to /dev/null is beyond me...
 */
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
        fprintf(stderr, ">> %s\n", line_start);
        parse_buffer(line_start);
        ++line_end;
        buf_used -= line_end - line_start;
        line_start = line_end;
    }

    memmove(inbuf, line_start, buf_used);
}

/* We should globally declare this fd since
 * it's going to be important. */
void *
start_listener(void * unused)
{
    (void)unused;

    fprintf(stderr, "in start_listener\n");

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
        fprintf(stderr, "!tests_done\n");
        fox_read(client_sock_fd);
    }

    fprintf(stderr, "start_listener: close()\n");

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
