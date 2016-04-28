/*
 *   foxbot.c -- April 26 2016 14:31:24 EST
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

#include <assert.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "conf.h"
#include "foxbot.h"
#include "foxsignal.h"
#include "foxmemory.h"
#include "stdinc.h"

void raw(char *fmt, ...);

struct bot_t bot; /* That's me */
volatile sig_atomic_t quitting;

/* External helper functions */
bool
is_registered(void)
{
    return bot.registered;
}

/* Socket functions */
static int
create_and_bind(void)
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /* TODO: Un-hardcode this. */
    if (getaddrinfo(botconfig.host, botconfig.port, &hints, &bot.hil) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    bot.fd = socket(bot.hil->ai_family, bot.hil->ai_socktype, bot.hil->ai_protocol);
    if (bot.fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    return 0;
}

static int
establish_link(void)
{
    int status, flags;

    status = connect(bot.fd, bot.hil->ai_addr, bot.hil->ai_addrlen);

    if (status == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    if(-1 == (flags = fcntl(bot.fd, F_GETFL, 0)))
        flags = 0;

    fcntl(bot.fd, F_SETFL, flags | O_NONBLOCK);

    raw("NICK %s\n", botconfig.nick);
    raw("USER %s 0 0 :%s\n", botconfig.ident, botconfig.realname);

    return 0;
}

static void
sockwrite(const char *buf)
{
    ssize_t writeval = strlen(buf);
    ssize_t retval = write(bot.fd, buf, writeval);

    if (retval == -1) {
        fprintf(stderr, "Write error: %s\n", strerror(errno));
        quitting = 1;
        return;
    }

    /* Warn? */
    if (writeval != retval) {
        fprintf(stderr, "Write error: Data content count mis-match\n");
        return;
    }
}

void
raw(char *fmt, ...) {
    char sbuf[MAX_IRC_BUF] = { 0 };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(sbuf, MAX_IRC_BUF, fmt, ap);
    va_end(ap);

    printf("<< %s", sbuf);
    sockwrite(sbuf);
}

/* IRC related socket helpers */
void
privmsg(const char *target, const char *message) {
    char buf[MAX_IRC_BUF];
    snprintf(buf, sizeof(buf), "PRIVMSG %s :%s\n", target, message);
    raw(buf);
}

void
join(const char *channel) {
    char buf[MAX_IRC_BUF];
    snprintf(buf, sizeof(buf), "JOIN %s\n", channel);
    raw(buf);
}

void
do_quit(const char *message) {
    char buf[MAX_IRC_BUF];
    snprintf(buf, sizeof(buf), "QUIT :%s\n", message);
    raw(buf);
}

void
do_error(char *line, ...) {
    char buf[MAX_IO_BUF] = { 0 };
    va_list ap;
    va_start(ap, line);
    vsnprintf(buf, MAX_IO_BUF, line, ap);
    va_end(ap);

    if (bot.registered)
        privmsg(botconfig.channel, buf);
    else
        fprintf(stderr, "%s\n", buf);
}

/* Command parser functions */
static int
do_set_enum(const char *command, enum commands ecmd) {
    if (strcmp(bot.msg->command, command) == 0) {
        bot.msg->ctype = ecmd;
        return 1;
    }

    return 0;
}

/* Do the messy string compare stuff now, that way
 * later on all we can check for is the ctype. */
static void
set_command_enum(void)
{
    assert(bot.msg->command);

    size_t ii;
    size_t n = strlen(bot.msg->command);
    unsigned int digits = 0;

    for (ii = 0; bot.msg->command[ii]; ii++)
    if (isdigit(bot.msg->command[ii]))
        digits++;

    if (digits == n) {
        bot.msg->ctype = NUMERIC;
        bot.msg->numeric = atoi(bot.msg->command);
        return;
    }

    if (do_set_enum("PRIVMSG", PRIVMSG)) return;
    if (do_set_enum("NOTICE",  NOTICE) ) return;
    if (do_set_enum("PING",    PING)   ) return;
    if (do_set_enum("PONG",    PONG)   ) return;
    if (do_set_enum("JOIN",    JOIN)   ) return;
    if (do_set_enum("PART",    PART)   ) return;
    if (do_set_enum("QUIT",    QUIT)   ) return;
    if (do_set_enum("MODE",    MODE)   ) return;
    if (do_set_enum("KICK",    KICK)   ) return;
    if (do_set_enum("ERROR",   ERROR)  ) return;

    do_error("Unhandled command: %s", bot.msg->command);
}

static bool
get_nuh(char *src)
{
    char *n, *u, *h;
    if (!(strstr(src, "!") && strstr(src, "@")))
        return false;

    /* I hate this */
    n = strtok(src, "!");
    if (!n) goto fail;
    bot.msg->from->nick = xstrdup(n);
    u = strtok(NULL, "!");
    if (!u) goto fail;
    h = strtok(u, "@");
    if (!h) goto fail;
    bot.msg->from->ident = xstrdup(h);
    h = strtok(NULL, "@");
    if (!h) goto fail;
    bot.msg->from->host = xstrdup(h);
    return true;
fail:
    do_error("Error parsing n!u@h %s", src);
    return false;
}

static void
free_message(void)
{
    xfree(bot.msg->buffer);
    bot.msg->buffer = NULL;

    xfree(bot.msg->source);
    bot.msg->source = NULL;

    xfree(bot.msg->command);
    bot.msg->command = NULL;

    xfree(bot.msg->target);
    bot.msg->target = NULL;

    xfree(bot.msg->params);
    bot.msg->params = NULL;

    /* zero out the from field */
    xfree(bot.msg->from->nick);
    bot.msg->from->nick = NULL;

    xfree(bot.msg->from->ident);
    bot.msg->from->ident = NULL;

    xfree(bot.msg->from->host);
    bot.msg->from->host = NULL;
}

void
hook_numeric(void)
{
    switch(bot.msg->numeric) {
    case 001: /* RPL_WELCOME */
        bot.registered = true;
        join(botconfig.channel);
        break;
    case 421: /* ERR_UNKNOWNCOMMAND */
        do_error(bot.msg->buffer);
        break;
    default:
        break;
    }
}

void
call_hooks(void)
{
    if (bot.msg->ctype == NUMERIC)
        hook_numeric();
}

char *
fox_strsep(char **stringp, const char *delim)
{
    char *p = *stringp;
    if (p) {
        char *q = p + strcspn(p, delim);
        *stringp = *q ? q + 1 : NULL;
        *q = '\0';
    }
    return p;
}

void
parse_line(const char *line)
{
    unsigned int i = 0, ii;
    int params = 1;
    char *token, *string, *tofree;

    free_message();

    bot.msg->buffer = xstrdup(line);

    for (ii = 0; line[ii] != '\0'; ii++)
    if (line[ii] == ' ')
        params++;

    tofree = string = xstrdup(line);

    while (((token = fox_strsep(&string, " ")) != NULL) && i < 3) {
        if (params < 3) {
            if (i == 0 && strncmp(token, "PING", 4) == 0) {
                size_t n = strlen(line);
                char tmp[MAX_IRC_BUF];
                memcpy(tmp, line, n);
                tmp[1] = 'O';
                tmp[n] = '\n';
                tmp[n + 1] = '\0';
                raw(tmp);
                goto end;
            } else {
                do_error("(2) Invalid line detected. Skipping... %s", line);
                goto end;
            }
            i++;
            continue;
        }

        switch(i) {
        case 0:
            if (token[0] == ':') {
                bot.msg->source = xstrdup(token + 1);
                if (!get_nuh(token + 1))
                    bot.msg->from_server = true;
            } else {
                if (strncmp(token, "ERROR", 5) == 0) {
                    quitting = 1;
                    goto end;
                }
                bot.msg->is_invalid = true;
                do_error("Invalid line detected. Skipping... %s", line);
                goto end;
            }
            break;
        case 1:
            bot.msg->command = xstrdup(token);
            set_command_enum();
            break;
        case 2:
            bot.msg->target = xstrdup(token);
            break;
        }

        i++;
    }

    if (params > 2) {
        /* Messy, gets the parameters from the IRCd command by setting our
         * params to start at the point after the source, spaces, command, etc.
         */
        size_t len = strlen(bot.msg->source + 1) + 1 + strlen(bot.msg->command) + 1 + strlen(bot.msg->target) + 3;
        if (strlen(line) > len)
            bot.msg->params = xstrdup(line + len);
        else if (bot.msg->ctype != JOIN && bot.msg->ctype != PART)
            printf("hmm? (%zu) > %zu - Command: %s\n", strlen(line), len, bot.msg->command);
    }

    call_hooks();

end:
    xfree(tofree);
}

/* Main IO loop */
static void
io(void)
{
    char inbuf[MAX_IO_BUF], innbuf[MAX_IO_BUF];
    size_t inbuf_used = 0, buf_used = 0;
    size_t buf_remain = sizeof(inbuf) - inbuf_used;

    if (buf_remain == 0) {
        fprintf(stderr, "Line exceeded buffer length\n");
        quitting = 1;
        return;
    }

    ssize_t rv = recv(bot.fd, (void *)&inbuf[inbuf_used], buf_remain, 0);

    if (rv == 0) {
        fprintf(stderr, "Remote host closed the connection\n");
        quitting = 1;
        return;
    }

    /* no data for now, call back when the socket is readable */
    if (rv < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return;

    if (rv < 0) {
        fprintf(stderr, "Read error: %s\n", strerror(errno));
        quitting = 1;
        return;
    }

    buf_used += rv;

    /* Scan for newlines in the line buffer; we're careful here to deal with embedded \0s
     * a broken server may send, as well as only processing lines that are complete.
     */
    char *line_start = inbuf;
    char *line_end;
    while ((line_end = memchr(line_start, '\n', buf_used - (line_start - inbuf))))
    {
        *line_end = 0;
        /* Straight out of RFC */
        assert(strlen(line_start) <= MAX_IRC_BUF);
        printf(">> %s\n", line_start);
        parse_line(line_start);
        line_start = line_end + 1;
    }

    /* Shift buffer down so the unprocessed data is at the start */
    buf_used -= (line_start - inbuf);
    memmove(innbuf, line_start, buf_used);
}

int
main(/*int argc, char **argv*/)
{
    bot.msg = xmalloc(sizeof(*bot.msg));
    bot.msg->from = xmalloc(sizeof(*bot.msg->from));

    read_conf_file();
    setup_signals();
    create_and_bind();
    establish_link();

    while (!quitting) {
        io();
    }

    if (is_registered()) {
        char buf[MAX_IRC_BUF];
        const char *reason = "<unknown>";
        switch (quitting) {
        case 1:
            reason = "QUIT";
            break;
        case 2:
            reason = "SIGINT";
            break;
        }
        snprintf(buf, sizeof(buf), "Exiting due to %s", reason);
        do_quit(buf);
    }

    return 0;
}
