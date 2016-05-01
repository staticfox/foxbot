/*
 *   channel.c -- April 30 2016 00:57:23 EST
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

#include <assert.h>
#include <string.h>

#include "memory.h"
#include "channel.h"

/** Global channel cache */
static dlink_list *channels;

void
init_channels(void)
{
    if (!channels)
        channels = dlist_create();
}

struct channel_t *
create_channel(const char *name)
{
    struct channel_t *channel = xmalloc(sizeof(*channel));
    channel->name = xstrdup(name);
    channel->users = dlist_create();

    dlink_insert(channels, channel);

    return channel;
}

void
add_user_to_channel(struct channel_t *channel, struct user_t *user)
{
    assert(channel != NULL);
    assert(channel->users != NULL);
    assert(user != NULL);
    dlink_insert(channel->users, user);
}

void
delete_channel(const char *name)
{
    dlink_node *node = NULL;
    struct channel_t *channel = NULL;

    DLINK_FOREACH(node, channels->head) {
        if (strcmp(((struct channel_t *)node->data)->name, name) == 0) {
            channel = (struct channel_t *)node->data;
            xfree(channel->name);
            xfree(channel->modes);
            dlink_delete(node, channels);
            return;
        }
    }

    /* Should never be here */
    assert(0);
}

void
delete_channel_s(struct channel_t *channel)
{
    dlink_node *node = NULL;

    if ((node = dlink_find(channels, channel))) {
        xfree(channel->name);
        xfree(channel->modes);
        dlink_delete(node, channels);
        return;
    }

    /* Should never be here */
    assert(0);
}

struct channel_t *
find_channel(const char *name)
{
    dlink_node *node = NULL;
    DLINK_FOREACH(node, channels->head)
        if (strcmp(((struct channel_t *)node->data)->name, name) == 0)
            return (struct channel_t *)node->data;
    return NULL;
}
