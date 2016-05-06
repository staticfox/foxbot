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
    static const struct user_t empty_user;
    struct user_t *user = xmalloc(sizeof(*user));
    *user = empty_user;
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

    const char *nick = strtok(src, "!");
    if (!nick) goto fail;

    char *ident = strtok(NULL, "!");
    if (!ident) goto fail;
    ident = strtok(ident, "@");
    if (!ident) goto fail;
    const char *host = strtok(NULL, "@");
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
    struct user_t *user = xmalloc(sizeof(*user));
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

    const char *nick = strtok(src, "!");
    if (!nick) goto fail;

    struct user_t *user = find_nick(nick);
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
