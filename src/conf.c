/*
 *   conf.c -- April 27 2016 19:45:28 EST
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
#include <string.h>

#include <foxbot/conf.h>
#include <foxbot/foxbot.h>

struct conf_parser_context conf_parser_ctx;
struct botconfig_entry botconfig;

extern int yyparse();

static void
clear_conf(void)
{
    xfree(botconfig.nick);
    botconfig.nick = NULL;
    xfree(botconfig.ident);
    botconfig.ident = NULL;
    xfree(botconfig.host);
    botconfig.host = NULL;
    xfree(botconfig.port);
    botconfig.port = NULL;
    xfree(botconfig.channel);
    botconfig.channel = NULL;
    xfree(botconfig.realname);
    botconfig.realname = NULL;
}

static void
set_default_conf(void)
{
    clear_conf();
    botconfig.nick = xstrdup("foxbot");
    botconfig.ident = xstrdup("foxbot");
    botconfig.host = xstrdup("misconfigured.host");
    botconfig.port = xstrdup("-4");
    botconfig.channel = xstrdup("#misconfigured");
    botconfig.realname = xstrdup(":3");
}

static void
override_test_port(void)
{
    if (bot.test_port > -1) {
        xfree(botconfig.port);
        char buf[65535];
        snprintf(buf, sizeof(buf), "%d", bot.test_port);
        botconfig.port = xstrdup(buf);
    }
}

static void
read_conf(FILE *file)
{
    set_default_conf();
    conf_parser_ctx.pass = 1;
    yyparse();
    rewind(file);
    conf_parser_ctx.pass = 2;
    clear_conf();
    yyparse(file);
    override_test_port();
}

void
read_conf_file(void)
{
    const char *filename;

    if (conf_parser_ctx.config_file_path != NULL)
        filename = conf_parser_ctx.config_file_path;
    else
        filename = "foxbot.conf";

    if ((conf_parser_ctx.conf_file = fopen(filename, "r")) == NULL) {
        do_error("Unable to read %s: %s", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    read_conf(conf_parser_ctx.conf_file);
    fclose(conf_parser_ctx.conf_file);
}

void
yyerror(const char *message)
{
    if (conf_parser_ctx.pass != 1)
        return;

    do_error((char *)message);
    exit(EXIT_FAILURE);
}
