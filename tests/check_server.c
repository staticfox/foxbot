/*
 *   check_server.c -- May 1 2016 10:58:00 EDT
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

#include <config.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <foxbot/conf.h>
#include <foxbot/memory.h>
#include <foxbot/message.h>
#include <foxbot/rope.h>
#include <foxbot/foxbot.h>

#include "check_server.h"

static const struct sockaddr_in SOCKADDR_IN_EMPTY;

static int sockfd = -1;

enum check_commands {
    CHECK_INVALID,
    CHECK_NICK,
    CHECK_USER,
    CHECK_PONG,
    CHECK_QUIT,
    CHECK_JOIN
};

static bool got_nick, got_user, connected;
static char *check_nick, *check_user;
char *last_buffer;
static int notification_pipe[2] = {-1, -1};
static tsrope write_queue = TSROPE_EMPTY;

void
wait_for_server_notification(void)
{
    char c;
    if (read(notification_pipe[0], &c, 1) < 1) {
        fprintf(stderr, "Read error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void
trigger_server_notification(void)
{
    if (write(notification_pipe[1], "\0", 1) < 1) {
        fprintf(stderr, "[Server] Write error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void
ts6_burst(void)
{
    fox_write(":ircd.staticfox.net NOTICE * :*** Ident disabled, not checking ident\r\n");
    fox_write(":ircd.staticfox.net NOTICE * :*** Looking up your hostname...\r\n");
    fox_write(":ircd.staticfox.net CAP * LS :account-notify account-tag away-notify cap-notify chghost echo-message extended-join invite-notify multi-prefix sasl server-time tls userhost-in-names\r\n");
    fox_write(":ircd.staticfox.net 001 %s :Welcome to the StaticFox Internet Relay Chat Network %s\r\n", check_nick, check_nick);
    fox_write(":ircd.staticfox.net 002 %s :Your host is ircd.staticfox.net[ircd.staticfox.net/9990], running version generic-ircd-1\r\n", check_nick);
    fox_write(":ircd.staticfox.net 003 %s :This server was created Sat Apr 30 2016 at 21:34:09 EDT\r\n", check_nick);
    fox_write(":ircd.staticfox.net 004 %s ircd.staticfox.net generic-ircd-1 DQRSZagiloswz CFILPQbcefgijklmnopqrstvz bkloveqjfI\r\n", check_nick);
    fox_write(":ircd.staticfox.net 005 %s CPRIVMSG CNOTICE MONITOR=100 WHOX ETRACE SAFELIST ELIST=CTU KNOCK FNC CHANTYPES=&# EXCEPTS INVEX :are supported by this server\r\n", check_nick);
    fox_write(":ircd.staticfox.net 005 %s CHANMODES=eIbq,k,flj,CFLPQcgimnprstz CHANLIMIT=&#:15 PREFIX=(ov)@+ MAXLIST=bqeI:100 MODES=4 NETWORK=StaticFox STATUSMSG=@+ CALLERID=g CASEMAPPING=rfc1459 NICKLEN=30 MAXNICKLEN=31 CHANNELLEN=50 :are supported by this server\r\n", check_nick);
    fox_write(":ircd.staticfox.net 005 %s TOPICLEN=390 DEAF=D TARGMAX=NAMES:1,LIST:1,KICK:1,WHOIS:1,PRIVMSG:4,NOTICE:4,ACCEPT:,MONITOR: CLIENTVER=3.0 :are supported by this server\r\n", check_nick);
    fox_write(":ircd.staticfox.net 251 %s :There are 0 users and 18 invisible on 2 servers\r\n", check_nick);
    fox_write(":ircd.staticfox.net 252 %s 1 :IRC Operators online\r\n", check_nick);
    fox_write(":ircd.staticfox.net 254 %s 4 :channels formed\r\n", check_nick);
    fox_write(":ircd.staticfox.net 255 %s :I have 3 clients and 1 servers\r\n", check_nick);
    fox_write(":ircd.staticfox.net 265 %s 3 4 :Current local users 3, max 4\r\n", check_nick);
    fox_write(":ircd.staticfox.net 266 %s 18 19 :Current global users 18, max 19\r\n", check_nick);
    fox_write(":ircd.staticfox.net 250 %s :Highest connection count: 5 (4 clients) (30 connections received)\r\n", check_nick);
    fox_write(":ircd.staticfox.net 375 %s :- ircd.staticfox.net Message of the Day -\r\n", check_nick);
    fox_write(":ircd.staticfox.net 372 %s :- Not an important MOTD\r\n", check_nick);
    fox_write(":ircd.staticfox.net 376 %s :End of /MOTD command.\r\n", check_nick);
}

static void
inspircd_burst(void)
{
    fox_write(":ircd.staticfox.net NOTICE Auth :*** Looking up your hostname...\r\n");
    fox_write(":ircd.staticfox.net NOTICE Auth :*** Found your hostname (localhost.localdomain) -- cached\r\n");
    fox_write(":ircd.staticfox.net NOTICE Auth :Welcome to StaticFox!\r\n");
    fox_write(":ircd.staticfox.net 001 %s :Welcome to the StaticFox IRC Network %s!%s@127.0.0.1\r\n", check_nick, check_nick, check_user);
    fox_write(":ircd.staticfox.net 002 %s :Your host is ircd.staticfox.net, running version InspIRCd-2.0\r\n", check_nick);
    fox_write(":ircd.staticfox.net 003 %s :This server was created 09:42:41 Sep 26 2015\r\n", check_nick);
    fox_write(":ircd.staticfox.net 004 %s ircd.staticfox.net InspIRCd-2.0 BGcgioswx BCGINOPQYabceghiklmnopqstuv IYabeghkloqv\r\n", check_nick);
    fox_write(":ircd.staticfox.net 005 %s AWAYLEN=200 CALLERID=g CASEMAPPING=rfc1459 CHANMODES=Ibeg,k,l,BCGNOPQcimnpstu CHANNELLEN=64 CHANTYPES=# CHARSET=ascii ELIST=MU EXCEPTS=e EXTBAN=,BCNOQcm FNC INVEX=I KICKLEN=255 :are supported by this server\r\n", check_nick);
    fox_write(":ircd.staticfox.net 005 %s MAP MAXBANS=60 MAXCHANNELS=20 MAXPARA=32 MAXTARGETS=20 MODES=20 NETWORK=StaticFox NICKLEN=31 OVERRIDE PREFIX=(Yqaohv)!~&@%%+ STATUSMSG=!~&@%%+ TOPICLEN=307 VBANLIST :are supported by this server\r\n", check_nick);
    fox_write(":ircd.staticfox.net 005 %s WALLCHOPS WALLVOICES :are supported by this server\r\n", check_nick);
    fox_write(":ircd.staticfox.net 042 %s 504AAAAAF :your unique ID\r\n", check_nick);
    fox_write(":ircd.staticfox.net 375 %s :ircd.staticfox.net message of the day\r\n", check_nick);
    fox_write(":ircd.staticfox.net 372 %s :- Not an important MOTD\r\n", check_nick);
    fox_write(":ircd.staticfox.net 376 %s :End of message of the day.\r\n", check_nick);
    fox_write(":ircd.staticfox.net 251 %s :There are 1 users and 0 invisible on 1 servers\r\n", check_nick);
    fox_write(":ircd.staticfox.net 254 %s 0 :channels formed\r\n", check_nick);
    fox_write(":ircd.staticfox.net 255 %s :I have 1 clients and 0 servers\r\n", check_nick);
    fox_write(":ircd.staticfox.net 265 %s :Current Local Users: 1  Max: 2\r\n", check_nick);
    fox_write(":ircd.staticfox.net 266 %s :Current Global Users: 1  Max: 2\r\n", check_nick);
}

/* Simulate an introduction to the IRC server. Wheeeeee! */
void
do_burst(int i)
{
    switch(i) {
    case 0:
        ts6_burst();
        break;
    case 1:
        inspircd_burst();
        break;
    default:
        ts6_burst();
        break;
    }

    fox_write(":%s MODE %s :+i\r\n", check_nick, check_nick);
    /* Needed to give the bot an idea of what its n!u@h is */
    fox_write(":%s!~%s@127.0.0.1 JOIN %s\r\n", check_nick, check_user, botconfig.channel);
    fox_write(":ircd.staticfox.net 353 %s = %s :%s\r\n", check_nick, botconfig.channel, check_nick);
    fox_write(":ircd.staticfox.net 366 %s %s :End of /NAMES list.\r\n", check_nick, botconfig.channel);
    fox_write(":%s!~%s@127.0.0.1 JOIN %s\r\n", check_nick, check_user, botconfig.debug_channel);
    fox_write(":ircd.staticfox.net 353 %s = %s :%s\r\n", check_nick, botconfig.debug_channel, check_nick);
    fox_write(":ircd.staticfox.net 366 %s %s :End of /NAMES list.\r\n", check_nick, botconfig.debug_channel);
}

void
send_broken_uint_value(void)
{
    fox_write(":ircd.staticfox.net 005 %s NICKLEN=-30 :are supported by this server\r\n", check_nick);
}

/* More or less only really needed for NICK and USER.
 * The rest we can determine what was sent as we will
 * be the ones sending the commands.
 */
static void
parse_buffer(const char *buf)
{
    enum check_commands cmd = CHECK_INVALID;
    unsigned int i = 0, ii;
    int params = 1;
    char *token, *string, *tofree;
    static char *l_params = NULL;

    for (ii = 0; buf[ii] != '\0'; ii++)
        if (buf[ii] == ' ')
            params++;

    xfree(last_buffer);
    last_buffer = xstrdup(buf);

    tofree = string = xstrdup(buf);

    while (((token = fox_strsep(&string, " ")) != NULL) && i < 3) {
        switch(i) {
        case 0:
            if (strcmp(token, "NICK") == 0)
                cmd = CHECK_NICK;
            else if (strcmp(token, "USER") == 0)
                cmd = CHECK_USER;
            else if (strcmp(token, "PONG") == 0)
                cmd = CHECK_PONG;
            else if (strcmp(token, "ASDF") == 0)
                fox_write(":ircd.staticfox.net 421 %s ASDF :Unknown command\r\n", check_nick);
            else if (strcmp(token, "QUIT") == 0)
                cmd = CHECK_QUIT;
            else if (strcmp(token, "JOIN") == 0)
                cmd = CHECK_JOIN;
            break;
        case 1:
            if (cmd == CHECK_NICK) {
                check_nick = xstrdup(token);
                got_nick = true;
                goto end;
            } else if (cmd == CHECK_USER) {
                check_user = xstrdup(token);
                got_user = true;
                goto end;
            } else if (cmd == CHECK_PONG) {
                if (strcmp(token, ":ircd.staticfox.net") != 0) {
                    fprintf(stderr, "invalid PONG message\n");
                    exit(EXIT_FAILURE);
                }
                goto end;
            } else if (cmd == CHECK_JOIN) {
                //fox_write(":%s!~%s@127.0.0.1 JOIN %s\r\n", check_nick, check_user, token);
                goto end;
            }
            break;
        }
        i++;
    }

    if (params > 2) {
        size_t len = 0;
        if (cmd == CHECK_QUIT)
            len = 6;
        if (strlen(buf) > len)
            l_params = xstrdup(buf + len);
    }

    if (cmd == CHECK_QUIT)
        fox_write("ERROR :Closing Link: 127.0.0.1 (Quit: %s)\r\n", l_params);

end:
    xfree(tofree);

    if (got_nick && got_user && !connected) {
        connected = true;
    }

    xfree(l_params);
    l_params = NULL;

}

void
fox_write(const char *line, ...)
{
    char buf[MAX_IRC_BUF] = {0};

    va_list ap;
    va_start(ap, line);
    vsnprintf(buf, MAX_IRC_BUF, line, ap);
    va_end(ap);

    append_tsrope(&write_queue, alloc_rope_segment(buf, strlen(buf) + 1));
}

void
fox_shutdown(void)
{
    char nothing;
    append_tsrope(&write_queue, alloc_rope_segment(&nothing, 0));
}

/* Write thread. */
static void *
start_writer(void *client_sock_fd_ptr)
{
    const int client_sock_fd = *(const int *)client_sock_fd_ptr;
    while (1) {
        struct rope_segment *const s = waitshift_tsrope(&write_queue);
        if (!s->len)
            break;
        if (write(client_sock_fd, s->data, s->len - 1) < 0) {
            fprintf(stderr, "[Server] Write error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        xfree(s);
    }
    if (shutdown(client_sock_fd, SHUT_WR)) {
        fprintf(stderr, "[Server] Cannot shutdown: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return NULL;
}

/* Function pointer as this starts as a
 * thread from check_foxbot. */
void *
start_listener(void *unused)
{
    (void)unused;

    struct sockaddr_in cli_addr = SOCKADDR_IN_EMPTY;
    socklen_t clilen;

    clilen = sizeof(cli_addr);

    const int client_sock_fd =
        accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if (client_sock_fd < 0) {
        fprintf(stderr, "[Server] Accept error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    pthread_t thread;
    if (pthread_create(&thread, NULL, start_writer, (void *)&client_sock_fd)) {
        fprintf(stderr, "[Server] Cannot create writer thread: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    io_state io;
    init_io(&io, client_sock_fd, MAX_IRC_BUF);
    char *line;
    while ((line = io_simple_readline(&io, "[Server] "))) {
        if (line[0] == '#')
            trigger_server_notification();
        else
            parse_buffer(line);
    }
    reset_io(&io);

    fox_shutdown();
    if (pthread_join(thread, NULL)) {
        fprintf(stderr, "[Server] Cannot join writer thread: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    close(client_sock_fd);

    return NULL;
}

void
shutdown_test_server(void)
{
    clear_tsrope(&write_queue);
    close(sockfd);
    close(notification_pipe[0]);
    close(notification_pipe[1]);
}

static int
bind_socket(int sockfd)
{
    int nport = 43210;

    struct sockaddr_in serv_addr = SOCKADDR_IN_EMPTY;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    /* Try for 75 ports to bind to, if we run out of ports
     * then oh well, the machine probably has bigger issues
     * to deal with \o/
     */
    for (int i = 0; i < 75; i++) {
        serv_addr.sin_port = htons(++nport);
        if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
            return nport;
        }
    }

    /* Shouldn't be here, but I guess we couldn't find an available
     * port to bind to :( */
    fprintf(stderr, "[Server] Bind error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

int
setup_test_server(void)
{
    int set = 1;

    if (pipe(notification_pipe)) {
        fprintf(stderr, "[Server] Cannot create pipe: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        fprintf(stderr, "[Server] Socket error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* This tells the kernel that we are willing to reuse the socket
     * regardless if there is lingering information.
     * Fixes https://github.com/staticfox/foxbot/issues/17
     */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) == -1) {
        fprintf(stderr, "[Server] setsockopt error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    const int port = bind_socket(sockfd);
    listen(sockfd, 5);
    return port;
}
