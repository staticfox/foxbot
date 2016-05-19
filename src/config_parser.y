/*
 *   config_parser.y -- April 27 2016 18:39:58 EDT
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

%token T_BOT
%token T_CHANNEL
%token T_HOST
%token T_IDENT
%token T_MODULE
%token T_NICK
%token T_DEBUG_CHANNEL
%token T_PORT
%token T_REALNAME
%token T_STRING

%type <string> T_STRING
%%

conf: | conf conf_item;
conf_item: bot_entry | error ';' | error '}' ;

/* Bot config */
bot_entry: T_BOT  '{' bot_items '}' ';' ;

bot_items: bot_items bot_item | bot_item;
bot_item:  bot_nick |
           bot_ident |
           bot_host |
           bot_port |
           bot_debug_channel |
           bot_channel |
           bot_realname |
           bot_module |
           error ';' ;

bot_nick: T_NICK '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.nick);
    botconfig.nick = xstrdup(yylval.string);
};

bot_ident: T_IDENT '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.ident);
    botconfig.ident = xstrdup(yylval.string);
};

bot_host: T_HOST '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.host);
    botconfig.host = xstrdup(yylval.string);
};

bot_port: T_PORT '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.port);
    botconfig.port = xstrdup(yylval.string);
};

bot_debug_channel: T_DEBUG_CHANNEL '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.debug_channel);
    botconfig.debug_channel = xstrdup(yylval.string);
}

bot_channel: T_CHANNEL '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.channel);
    botconfig.channel = xstrdup(yylval.string);
};

bot_realname: T_REALNAME '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.realname);
    botconfig.realname = xstrdup(yylval.string);
}

bot_module: T_MODULE '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    add_m_safe(yylval.string, CONF_MODULE);
}
