/*
 *   channel.c -- April 30 2016 00:57:23 EDT
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

#include <config.h>

#include <assert.h>
#include <string.h>
#include <strings.h>

#include <foxbot/channel.h>
#include <foxbot/foxbot.h>
#include <foxbot/memory.h>

/** Global channel cache */
static dlink_list channels;

void
clear_channels(void)
{
    DLINK_FOREACH(node, dlist_head(&channels))
        delete_channel_s(dlink_data(node));
}

struct channel_t *
create_channel(const char *name)
{
    static const struct channel_t CHANNEL_EMPTY;
    struct channel_t *channel = xmalloc(sizeof(*channel));
    *channel = CHANNEL_EMPTY;
    channel->name = xstrdup(name);

    dlink_insert(&channels, channel);

    return channel;
}

size_t
channel_count(void)
{
    return dlist_length(&channels);
}

struct member_t *
add_user_to_channel(struct channel_t *channel, struct user_t *user)
{
    static const struct member_t CHANNEL_MEMBER;
    struct member_t *member = NULL;
    assert(channel != NULL);
    assert(user != NULL);

    if ((member = channel_get_membership(channel, user)) != NULL) {
        do_error("Attempting to rejoin %s to %s.", user->nick, channel->name);
        return member;
    }

    member = xmalloc(sizeof(*member));
    *member = CHANNEL_MEMBER;
    member->user = user;

    user->number_of_channels++;
    dlink_insert(&channel->users, member);

    return member;
}

struct member_t *
channel_get_membership(struct channel_t *channel, struct user_t *user)
{
    DLINK_FOREACH(node, dlist_head(&channel->users)) {
        struct member_t *member = dlink_data(node);
        if (member->user == user)
            return member;
    }

    return NULL;
}

struct member_t *
channel_get_or_add_membership(struct channel_t *channel, struct user_t *user)
{
    struct member_t *const member = channel_get_membership(channel, user);
    if (member)
        return member;
    return add_user_to_channel(channel, user);
}

void
channel_remove_user(struct channel_t *channel, struct user_t *user)
{
    DLINK_FOREACH(u_node, dlist_head(&channel->users)) {
        struct member_t *member = dlink_data(u_node);
        if (member->user == user) {
            dlink_delete(u_node, &channel->users);
            break;
        }
    }

    if (--user->number_of_channels == 0 && user != bot.user)
        delete_user(user);
}

/* This function should really be renamed to
 * Ping timeout: 256 seconds */
void
channel_quit_user(struct user_t *user)
{
    DLINK_FOREACH(node, dlist_head(&channels))
        channel_remove_user(((struct channel_t *)dlink_data(node)), user);
}

void
delete_channel_s(struct channel_t *channel)
{
    dlink_node *node = NULL;

    if ((node = dlink_find(&channels, channel)) != NULL) {
        xfree(channel->name);
        xfree(channel->modes);

        /* Remove users from the channel. Also delete
         * their cache entry if need be */
        DLINK_FOREACH(u_node, dlist_head(&channel->users)) {
            struct member_t *member = dlink_data(u_node);
            if (--member->user->number_of_channels == 0 && member->user != bot.user)
                delete_user(member->user);
            dlink_delete(u_node, &channel->users);
        }

        xfree(channel);
        dlink_delete(node, &channels);
        return;
    }

    /* Should never be here */
    do_error("Received unknown channel struct for %p (%s)", channel, channel->name);
}

struct channel_t *
find_channel(const char *name)
{
    DLINK_FOREACH(node, dlist_head(&channels))
        if (strcasecmp(((struct channel_t *)dlink_data(node))->name, name) == 0)
            return (struct channel_t *)dlink_data(node);
    return NULL;
}
