/*
 *   user.c -- April 28 2016 23:29:44 EST
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
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "foxbot.h"
#include "memory.h"
#include "user.h"

/** The global user cache. */
static dlink_list *users;

void
init_users(void)
{
    if (!users)
        users = dlist_create();
}

void
make_me(const char *nick)
{
    struct user_t *user = xmalloc(sizeof(*user));
    user->nick = xstrdup(nick);

    dlink_insert(users, user);
}

struct user_t *
make_nuh(const char *n, const char *u, const char *h)
{
    struct user_t *user = xmalloc(sizeof(*user));
    user->nick  = xstrdup(n);
    user->ident = xstrdup(u);
    user->host  = xstrdup(h);

    dlink_insert(users, user);

    return user;
}

struct user_t *
get_user_by_nick(const char *nick)
{
    dlink_node *node = NULL;
    DLINK_FOREACH(node, users->head)
        if (strcmp(((struct user_t *)node->data)->nick, nick) == 0)
            return (struct user_t *)node->data;
    return NULL;
}

void
delete_user_by_nick(const char *nick)
{
    dlink_node *node = NULL;
    struct user_t *user = NULL;

    DLINK_FOREACH(node, users->head) {
        if (strcmp(((struct user_t *)node->data)->nick, nick) == 0) {
            user = (struct user_t *)node->data;
            xfree(user->nick);
            xfree(user->ident);
            xfree(user->host);
            dlink_delete(node, users);
            return;
        }
    }

    /* Should never be here */
    assert(0);
}

void
delete_user_by_struct(struct user_t *user)
{
    dlink_node *node = NULL;

    if ((node = dlink_find(users, user))) {
        xfree(user->nick);
        xfree(user->ident);
        xfree(user->host);
        dlink_delete(node, users);
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

    const char *nick = strtok(src, "!");
    if (!nick) goto fail;

    struct user_t *user = get_user_by_nick(nick);
    if (user != NULL)
        return user;

    char *ident = strtok(NULL, "!");
    if (!ident) goto fail;
    ident = strtok(ident, "@");
    if (!ident) goto fail;
    const char *host = strtok(NULL, "@");
    if (!host) goto fail;

    user = make_nuh(nick, ident, host);

    return user;
fail:
    do_error("Error parsing n!u@h %s", src);
    return NULL;
}
