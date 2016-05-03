/*
 *   conf.h -- April 27 2016 19:45:35 EST
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

#include "config.h"
#include "memory.h"

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
    char *channel;
    char *realname;
};

void read_conf_file(void);
void yyerror(const char *message);

extern struct conf_parser_context conf_parser_ctx;
extern struct botconfig_entry botconfig;

#endif
