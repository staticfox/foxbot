/*
 *   config_parser.y -- April 27 2016 18:39:58 EST
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

%{
#include "config.h"

#include <foxbot/conf.h>
#include <foxbot/memory.h>

int yylex(void);
%}

%union {
    int number;
    char *string;
}

%token BOT
%token CHANNEL
%token HOST
%token IDENT
%token NICK
%token PORT
%token REALNAME
%token STRING

%type <string> STRING
%%

conf: | conf conf_item;
conf_item: bot_entry | error ';' | error '}' ;

/* Bot config */
bot_entry: BOT  '{' bot_items '}' ';' ;

bot_items: bot_items bot_item | bot_item;
bot_item:  bot_nick |
           bot_ident |
           bot_host |
           bot_port |
           bot_channel |
           bot_realname |
           error ';' ;

bot_nick: NICK '=' STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.nick);
    botconfig.nick = xstrdup(yylval.string);
};

bot_ident: IDENT '=' STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.ident);
    botconfig.ident = xstrdup(yylval.string);
};

bot_host: HOST '=' STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.host);
    botconfig.host = xstrdup(yylval.string);
};

bot_port: PORT '=' STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.port);
    botconfig.port = xstrdup(yylval.string);
};

bot_channel: CHANNEL '=' STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.channel);
    botconfig.channel = xstrdup(yylval.string);
};

bot_realname: REALNAME '=' STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.realname);
    botconfig.realname = xstrdup(yylval.string);
}
