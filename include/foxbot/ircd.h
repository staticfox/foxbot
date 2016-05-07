/*
 *   ircd.h -- April 28 2016 19:47:26 EST
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

#ifndef FOX_IRCD_H_
#define FOX_IRCD_H_

#include <stdbool.h>

struct ircd_t {
    char *name;
    char *network;
    char *version;
    char *user_modes;
    unsigned int chan_limit;
    unsigned int nick_length;
    unsigned int channel_length;
    unsigned int topic_length;
    struct {
        bool whox;
        bool knock;
        bool invex;
        bool excepts;
        char *chanop_modes;
        char *prefix;
        char *chan_types;
    } supports;
    long int caps;
};

void parse_rpl_welcome(void);
void parse_rpl_myinfo(void);
void parse_rpl_isupport(void);

#endif
