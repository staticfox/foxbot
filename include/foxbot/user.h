/*
 *   user.h -- April 28 2016 23:31:08 EDT
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

#include <stdbool.h>
#include <stddef.h>

#include <foxbot/list.h>

/** A user object.  The object owns all of the member strings. */
struct user_t {
    char *nick;
    char *ident;
    char *host;
    char *gecos;
    char *flags;
    char *account;
    char *server;
    int hops;
    bool away;
    bool ircop;
    unsigned long idle;
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

/** Look up a user by pointer in the global user cache. */
bool user_pointer_valid(struct user_t *user);

/** Does `#find_nick` first to get the user, and failing that, it uses
 * `#make_nuh` to create the user. */
struct user_t * find_or_make_user(const char *nick,
                                  const char *ident,
                                  const char *host);

/** Delete a user. */
void delete_user(struct user_t *user);

/** Parse the nick, ident, and hostname (NUH) from a string.  The string is
    modified by this process. */
struct user_t * get_nuh(char *src);

/** Delete a user from the global user cache if we already have the user's
    struct. */
void delete_user(struct user_t *user);

#endif
