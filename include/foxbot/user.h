/*
 *   user.h -- April 28 2016 23:31:08 EST
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

#ifndef FOX_USER_H_
#define FOX_USER_H_

#include <stddef.h>

#include "list.h"

/** A user object.  The object owns the 3 member strings. */
struct user_t {
    char *nick;
    char *ident;
    char *host;
    size_t number_of_channels;
};

/** Create our user struct at 001 */
void make_me(const char *nick);

/** Returns the amount of users within the cache */
size_t user_count(void);

/** Set the user and hostname given the struct */
void set_uh(struct user_t *user, char *src);

/** Create a `#user_t` from the nick, ident, and hostname. */
struct user_t * make_nuh(const char *n, const char *u, const char *h);

/** Look up a user by nick in the global user cache. */
struct user_t * find_nick(const char *nick);

/** Delete a user. */
void delete_user(struct user_t *user);

/** Parse the nick, ident, and hostname (NUH) from a string.  The string is
    modified by this process. */
struct user_t * get_nuh(char *src);

/** Delete a user from the global user cache if we already have the user's
    struct. */
void delete_user(struct user_t *user);

#endif
