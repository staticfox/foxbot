/*
 *   list.c -- April 28 2016 18:22:29 EST
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "list.h"
#include "memory.h"

dlink_list *
dlist_create(void)
{
    return xmalloc(sizeof(dlink_list));
}

void
dlink_insert(dlink_list *list, void *data)
{
    assert(list);
    dlink_node *node = xmalloc(sizeof(dlink_node));
    node->data = data;
    node->prev = NULL;
    node->next = list->head;

    if (list->head)
        list->head->prev = node;
    else if (list->tail == NULL)
        list->tail = node;

    list->head = node;
    list->length++;
}

dlink_node *
dlink_find(dlink_list *list, void *data)
{
    dlink_node *node = NULL;
    DLINK_FOREACH(node, list->head)
        if (node->data == data)
            return node;

    return NULL;
}
