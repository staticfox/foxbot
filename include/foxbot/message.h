/*
 *   message.h -- April 29 2016 10:08:32 EST
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

#ifndef FOX_MESSAGE_H_
#define FOX_MESSAGE_H_

#include <stdbool.h>

#include "foxbot.h"

struct msg_t {
    enum commands ctype;
    unsigned int numeric;
    char *buffer;
    struct user_t *from;
    bool from_server;
    char *source;
    char *command;
    char *target;
    char *params;
    bool is_invalid;
};

void hook_numeric(void);
void hook_literal(void);
void call_hooks(void);
char * fox_strsep(char **stringp, const char *delim);
bool parse_line(const char *line);

#endif
