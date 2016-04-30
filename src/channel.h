/*
 *   channel.h -- April 30 2016 00:57:15 EST
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

#ifndef FOX_CHANNEL_H_
#define FOX_CHANNEL_H_

#include "list.h"

struct channel_t {
    char *name;
    char *modes;
};

/** Initialize the global channel cache.  Must be called before any of the
    functions in this module. */
void init_channels(void);

/** Create a `#channel_t` from the name. */
struct channel_t * create_channel(const char *name);

/** Delete a channel from the global channel cache if we already have the
    channel's struct. */
void delete_channel_s(struct channel_t *channel);

/** Delete a channel by name from the global channel cache. */
void delete_channel(const char *name);

/** Look up a channel by name in the global channel cache. */
struct channel_t * find_channel(const char *name);

#endif
