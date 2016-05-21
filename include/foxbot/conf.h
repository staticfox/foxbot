/*
 *   conf.h -- April 27 2016 19:45:35 EDT
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

#ifndef FOX_CONFIG_H_
#define FOX_CONFIG_H_

#include <stdio.h>

#include <foxbot/list.h>
#include <foxbot/memory.h>

enum conf_multiple_types {
    CONF_STANDARD_CHANNEL,
    CONF_DEBUG_CHANNEL,
    CONF_PLUGIN
};

enum admin_data_flag {
    CONF_ADMIN_NS,
    CONF_ADMIN_HOST
};

struct conf_multiple {
    enum conf_multiple_types type;
    char *name;
    char *key;
};

struct admin_struct_t {
    char *name;
    int access;
    dlink_list ns_accts;
    dlink_list hosts;
};

struct conf_parser_context {
    unsigned int pass;
    char *config_file_path;
    FILE *conf_file;
};

struct botconfig_entry {
    char *nick;
    char *ident;
    char *host;
    char *port;
    char *realname;
    char *password;
    dlink_list conf_modules;
    dlink_list admins;
};

void add_m_safe(const char *entry, enum conf_multiple_types type);
struct admin_struct_t * make_admin_conf(const char *entry);
void admin_add_data(struct admin_struct_t *entry, enum admin_data_flag type, const char *data);
void conf_set_ckey(const char *entry, enum conf_multiple_types type, const char *key);
void read_conf_file(void);
void yyerror(const char *message);

extern struct conf_parser_context conf_parser_ctx;
extern struct botconfig_entry botconfig;

#endif
