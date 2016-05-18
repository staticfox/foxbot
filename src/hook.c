/*
 *   hook.c -- May 18 2016 16:57:28 EDT
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

#include <foxbot/hook.h>
#include <foxbot/memory.h>

#include <stdio.h>
#include <string.h>

static dlink_list hooks;

void
add_hook(const char *const name, hook_func func)
{
    static const struct hook_t EMPTY_HOOK;
    struct hook_t *hook = xmalloc(sizeof(*hook));
    *hook = EMPTY_HOOK;
    hook->name = xstrdup(name);
    hook->func = func;
    dlink_insert(&hooks, hook);
    return;
}

size_t
hook_count(void)
{
    return dlist_length(&hooks);
}

void
exec_hook(const char *const name)
{
    const struct hook_t *hook = NULL;

    DLINK_FOREACH(node, dlist_head(&hooks)) {
        hook = (struct hook_t *)dlink_data(node);
        if (strcmp(hook->name, name) == 0)
            hook->func();
    }
}
