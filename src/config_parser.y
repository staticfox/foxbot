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
#include <stdbool.h>
#include <string.h>

#include "config.h"

#include <foxbot/conf.h>
#include <foxbot/memory.h>
#include <foxbot/parser.h>

int yylex(void);

static struct channel_state_t {
    char *name;
    char *key;
    int is_debug;
} channel_state;

static struct admin_state_t {
    char *name;
    int access;
    dlink_list nickserv;
    dlink_list hosts;
} admin_state;

static void
clear_cstate(void)
{
    static const struct channel_state_t EMPTY_CSTATE;
    xfree(channel_state.name);
    xfree(channel_state.key);
    channel_state = EMPTY_CSTATE;
}

static void
clear_adminstate(void)
{
    static const struct admin_state_t EMPTY_ADMIN;
    xfree(admin_state.name);

    DLINK_FOREACH(node, dlist_head(&admin_state.nickserv)) {
        xfree(dlink_data(node));
        dlink_delete(node, &admin_state.nickserv);
    }

    DLINK_FOREACH(node, dlist_head(&admin_state.hosts)) {
        xfree(dlink_data(node));
        dlink_delete(node, &admin_state.hosts);
    }

    admin_state = EMPTY_ADMIN;
}

%}

%union {
    int number;
    char *string;
}

%token T_ACCESS
%token T_ADMIN
%token T_BOT
%token T_NUMBER
%token T_DEBUG
%token T_HOST
%token T_IDENT
%token T_PLUGIN
%token T_NICK
%token T_NICKSERV
%token T_PASSWORD
%token T_PORT
%token T_REALNAME
%token T_CHANNEL
%token T_NAME
%token T_KEY
%token T_BOOL
%token T_STRING

%type <string> T_STRING
%%

conf: | conf conf_item;
conf_item: bot_entry | channel_entry | admin_entry | error ';' | error '}' ;

/* Bot config */
bot_entry: T_BOT  '{' bot_items '}' ';' ;

bot_items: bot_items bot_item | bot_item;
bot_item:  bot_nick |
           bot_ident |
           bot_host |
           bot_port |
           bot_realname |
           bot_plugin |
           bot_password |
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

bot_password: T_PASSWORD '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.password);
    botconfig.password = xstrdup(yylval.string);
}

bot_realname: T_REALNAME '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    xfree(botconfig.realname);
    botconfig.realname = xstrdup(yylval.string);
}

bot_plugin: T_PLUGIN '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    add_m_safe(yylval.string, CONF_PLUGIN);
}

/* Channel {} entries */
channel_entry: T_CHANNEL
{
    if (conf_parser_ctx.pass == 2)
        clear_cstate();
}  '{' channel_items '}' ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    if (channel_state.name == NULL)
        break;

    enum conf_multiple_types t;
    if (channel_state.is_debug)
        t = CONF_DEBUG_CHANNEL;
    else
        t = CONF_STANDARD_CHANNEL;

    add_m_safe(channel_state.name, t);
    if (channel_state.key != NULL)
        conf_set_ckey(channel_state.name, t, channel_state.key);
};

channel_items: channel_items channel_item | channel_item;
channel_item:  channel_name |
               channel_key |
               channel_debug |
               error ';' ;

channel_name: T_NAME '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    if (yylval.string[0])
        channel_state.name = xstrdup(yylval.string);
};

channel_key: T_KEY '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    if (yylval.string[0])
        channel_state.key = xstrdup(yylval.string);
}

channel_debug: T_DEBUG '=' T_BOOL ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    channel_state.is_debug = yylval.number;
}

/* Admin {} entries */
admin_entry: T_ADMIN
{
    if (conf_parser_ctx.pass == 2)
        clear_adminstate();
}  '{' admin_items '}' ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    if (admin_state.name == NULL)
        break;

    struct admin_struct_t *admin = make_admin_conf(admin_state.name);
    /* We already have the entry */
    if (admin == NULL)
        break;

    admin->access = admin_state.access;

    DLINK_FOREACH(node, dlist_head(&admin_state.nickserv))
        admin_add_data(admin, CONF_ADMIN_NS, dlink_data(node));

    DLINK_FOREACH(node, dlist_head(&admin_state.hosts))
        admin_add_data(admin, CONF_ADMIN_HOST, dlink_data(node));
};

admin_items: admin_items admin_item | admin_item;
admin_item:  admin_name |
             admin_nickserv |
             admin_host |
             admin_access |
             error ';' ;

admin_name: T_NAME '=' T_STRING ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    if (yylval.string[0])
        admin_state.name = xstrdup(yylval.string);
};

admin_nickserv: T_NICKSERV '=' T_STRING ';'
{
    bool found = false;

    if (conf_parser_ctx.pass != 2)
        break;

    DLINK_FOREACH(node, dlist_head(&admin_state.nickserv)) {
        if (fox_strcmp(dlink_data(node), yylval.string) == 0) {
            found = true;
            break;
        }
    }

    if (found)
        break;

    dlink_insert(&admin_state.nickserv, xstrdup(yylval.string));
};

admin_host: T_HOST '=' T_STRING ';'
{
    bool found = false;

    if (conf_parser_ctx.pass != 2)
        break;

    DLINK_FOREACH(node, dlist_head(&admin_state.hosts)) {
        if (fox_strcmp(dlink_data(node), yylval.string) == 0) {
            found = true;
            break;
        }
    }

    if (found)
        break;

    dlink_insert(&admin_state.hosts, xstrdup(yylval.string));
};

admin_access: T_ACCESS '=' T_NUMBER ';'
{
    if (conf_parser_ctx.pass != 2)
        break;

    admin_state.access = yylval.number;
};
