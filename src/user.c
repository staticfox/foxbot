/*
 *   user.c -- April 28 2016 23:29:44 EDT
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

#define _POSIX_C_SOURCE 200112L

#include <config.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <foxbot/list.h>
#include <foxbot/foxbot.h>
#include <foxbot/message.h>
#include <foxbot/memory.h>
#include <foxbot/user.h>

/** The global user cache. */
static dlink_list users;

void
make_me(const char *nick)
{
    static const struct user_t EMPTY_USER;
    struct user_t *user = xmalloc(sizeof(*user));
    *user = EMPTY_USER;
    user->nick = xstrdup(nick);
    user->number_of_channels = 0;

    bot.user = user;

    dlink_insert(&users, user);
}

size_t
user_count(void)
{
    return dlist_length(&users);
}

void
set_uh(struct user_t *user, char *src)
{
    if (!(strstr(src, "!") && strstr(src, "@")))
        return;

    char *lasts = NULL;
    const char *nick = strtok_r(src, "!", &lasts);
    if (!nick) goto fail;

    char *ident = strtok_r(NULL, "!", &lasts);
    if (!ident) goto fail;
    ident = strtok_r(ident, "@", &lasts);
    if (!ident) goto fail;
    const char *host = strtok_r(NULL, "@", &lasts);
    if (!host) goto fail;

    user->ident = xstrdup(ident);
    user->host  = xstrdup(host);
    return;
fail:
    do_error("Error parsing n!u@h %s", src);
}

struct user_t *
make_nuh(const char *n, const char *u, const char *h)
{
    static const struct user_t EMPTY_USER;
    struct user_t *user = xmalloc(sizeof(*user));
    *user = EMPTY_USER;
    user->nick  = xstrdup(n);
    user->ident = xstrdup(u);
    user->host  = xstrdup(h);
    user->number_of_channels = 0;

    dlink_insert(&users, user);

    return user;
}

struct user_t *
find_nick(const char *nick)
{
    DLINK_FOREACH(node, dlist_head(&users))
        if (strcmp(((struct user_t *)dlink_data(node))->nick, nick) == 0)
            return (struct user_t *)dlink_data(node);
    return NULL;
}

bool
user_pointer_valid(struct user_t *user)
{
    DLINK_FOREACH(node, dlist_head(&users)) {
        const struct user_t *const luser = (struct user_t *)dlink_data(node);
        if (luser == user)
            return true;
    }

    return false;
}

struct user_t *
find_or_make_user(const char *nick, const char *ident, const char *host)
{
    struct user_t *const user = find_nick(nick);
    if (user)
        return user;
    return make_nuh(nick, ident, host);
}

void
delete_user(struct user_t *user)
{
    dlink_node *node = NULL;

    if ((node = dlink_find(&users, user))) {
        if (bot.msg->from == user)
            bot.msg->from = NULL;
        xfree(user->nick);
        xfree(user->ident);
        xfree(user->host);
        xfree(user->gecos);
        xfree(user->flags);
        xfree(user->account);
        xfree(user->server);
        xfree(user);
        dlink_delete(node, &users);
        return;
    }

    /* Should never be here */
    assert(0);
}

struct user_t *
get_nuh(char *src)
{
    if (!(strstr(src, "!") && strstr(src, "@")))
        return NULL;

    char *lasts = NULL;
    const char *nick = strtok_r(src, "!", &lasts);
    if (!nick) goto fail;

    struct user_t *user = find_nick(nick);
    if (user != NULL)
        return user;

    char *ident = strtok_r(NULL, "!", &lasts);
    if (!ident) goto fail;
    ident = strtok_r(ident, "@", &lasts);
    if (!ident) goto fail;
    const char *host = strtok_r(NULL, "@", &lasts);
    if (!host) goto fail;

    user = make_nuh(nick, ident, host);

    return user;
fail:
    do_error("Error parsing n!u@h %s", src);
    return NULL;
}
