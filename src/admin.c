/*
 *   admin.c -- May 20 2016 17:10:34 EDT
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

#include <foxbot/conf.h>
#include <foxbot/parser.h>
#include <foxbot/user.h>

int
find_admin_access(const struct user_t *const user)
{
    /* Scan through all bot admins */
    DLINK_FOREACH(node, dlist_head(&botconfig.admins)) {

        const struct admin_struct_t *entry = dlink_data(node);

        DLINK_FOREACH(node2, dlist_head(&entry->ns_accts)) {
            if (user->account == NULL)
                continue;
            if (fox_strcmp(dlink_data(node2), user->account) == 0)
                return entry->access;
        }

        DLINK_FOREACH(node2, dlist_head(&entry->hosts)) {
            if (user->host == NULL)
                continue;
            if (fox_strcmp(dlink_data(node2), user->host) == 0)
                return entry->access;
        }
    }

    return 0;
}
