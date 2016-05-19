/*
 *   hook.h -- May 18 2016 17:07:08 EDT
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

#ifndef FOX_HOOK_H_
#define FOX_HOOK_H_

#include <foxbot/list.h>

typedef void * (*hook_func)(void);

struct hook_t {
    char *name;
    hook_func func;
};

void add_hook(const char *name, hook_func func);
void delete_hook(hook_func func);
void exec_hook(const char *nick);
size_t hook_count(void);

#endif
