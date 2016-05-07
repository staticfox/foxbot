/*
 *   list.h -- April 28 2016 21:18:53 EDT
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

/* These macros are basically swiped from the linux kernel
 * they are simple yet effective
 */

#ifndef FOX_LIST_H_
#define FOX_LIST_H_

#include <stddef.h>

/** Loop forward over a linked list starting at the given `head` node with a
  * new local variable named `pos`.  The current node can be deleted without
  * disrupting the loop. */
#define DLINK_FOREACH(pos, head) \
    for (dlink_node *pos = (head), *_##pos##_next = pos ? dlink_next(pos) : NULL; \
         pos; pos = _##pos##_next, _##pos##_next = pos ? dlink_next(pos) : NULL)

typedef struct dlink_node_ dlink_node;

struct dlink_node_ {
    void *data;
    dlink_node *prev;
    dlink_node *next;
};

typedef struct {
    dlink_node *head;
    dlink_node *tail;
    size_t length;
} dlink_list;

static inline size_t dlist_length(const dlink_list *list) { return list->length; }
static inline dlink_node *dlist_head(const dlink_list *list) { return list->head; }
static inline dlink_node *dlist_tail(const dlink_list *list) { return list->tail; }
static inline void *dlink_data(const dlink_node *node) { return node->data; }
static inline dlink_node *dlink_prev(const dlink_node *node) { return node->prev; }
static inline dlink_node *dlink_next(const dlink_node *node) { return node->next; }
void dlink_insert(dlink_list *list, void *data);
void dlink_delete(dlink_node *m, dlink_list *list);
dlink_node *dlink_find(const dlink_list *list, void *data);

#endif
