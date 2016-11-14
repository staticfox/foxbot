/*
 *   channel.h -- April 30 2016 00:57:15 EDT
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

#include <time.h>
#include <stddef.h>

#include <foxbot/list.h>
#include <foxbot/user.h>

#define CFLAG_USEROP    0x01
#define CFLAG_VOICE     0x02
#define CFLAG_HALFOP    0x04
#define CFLAG_OPERATOR  0x08
#define CFLAG_ADMIN     0x10
#define CFLAG_OWNER     0x20
#define CFLAG_SPECIAL   0x40

struct channel_t {
    char *name;
    char *modes;
    dlink_list users;
};

struct member_t {
    struct user_t *user;
    unsigned long int opstatus;
    time_t joined;
};

/** Clear the global channel cache. */
void clear_channels(void);

/** Create a `#channel_t` from the name. */
struct channel_t * create_channel(const char *name);

/** Returns the amount of channels within the cache */
size_t channel_count(void);

/** Add a `#user_t` to a channel. Returns a `#member_t`. */
struct member_t * add_user_to_channel(struct channel_t *channel, struct user_t *user);

/** Delete '#user_t' from all channels. */
void channel_quit_user(struct user_t *user);

/** Delete a '#user_t' from a '#channel_t' */
void channel_remove_user(struct channel_t *channel, struct user_t *user);

/** Returns a pointer is '#member_t' is in '#channel_t' */
struct member_t * channel_get_membership(struct channel_t *channel, struct user_t *user);

/** Tries `#channel_get_membership` first, and failing that, does
 * `#add_user_to_channel`. */
struct member_t * channel_get_or_add_membership(struct channel_t *channel,
                                                struct user_t *user);

/** Delete a channel from the global channel cache if we already have the
    channel's struct. */
void delete_channel_s(struct channel_t *channel);

/** Look up a channel by name in the global channel cache. */
struct channel_t * find_channel(const char *name);

static inline bool member_is_special(const struct member_t *member) { return member->opstatus & CFLAG_SPECIAL; }
static inline bool member_is_owner(const struct member_t *member) { return member->opstatus & CFLAG_OWNER; }
static inline bool member_is_admin(const struct member_t *member) { return member->opstatus & CFLAG_ADMIN; }
static inline bool member_is_op(const struct member_t *member) { return member->opstatus & CFLAG_OPERATOR; }
static inline bool member_is_halfop(const struct member_t *member) { return member->opstatus & CFLAG_HALFOP; }
static inline bool member_is_voice(const struct member_t *member) { return member->opstatus & CFLAG_VOICE; }
static inline bool member_is_userop(const struct member_t *member) { return member->opstatus & CFLAG_USEROP; }

#endif
