/*
 *   plugin.h -- May 18 2016 21:30:46 EDT
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

#ifndef FOX_PLUGIN_H
#define FOX_PLUGIN_H

#include <stdbool.h>

#define REG(name, params, access, func) register_command(name, params, access, func, &fox_plugin)

typedef void (*cmd_func)(const char *param);

struct command_t {
    char *name;
    int access;
    cmd_func func;
    unsigned params;
    struct plugin_t *plugin;
};

struct plugin_t {
    const char *name;
    bool (*register_func)(void);
    bool (*unregister_func)(void);
    char *version;
    char *description;
    char *author;
    char *build_time;
};

struct plugin_handle_t {
    void *dlobj;
    char *file_name;
    struct plugin_t *plugin;
};

struct plugin_handle_t * get_plugin_info(const char *const name);
void list_plugins(const char *nick);
void iregister_plugin(struct plugin_handle_t *plugin_handle, bool announce);
void iunload_plugin(const char *name, bool announce);
void iload_plugin(const char *name, bool announce);
void load_conf_plugins(void);
bool is_valid_plugin(const char *file, const struct plugin_t *p, bool announce);
void register_command(const char *command, const int params, const int access, const cmd_func func, struct plugin_t *p);
void unregister_command(const char *const command, const cmd_func func);
void register_default_commands(void);
struct command_t * find_command(const bool exec, const char *command, bool exact);
void exec_command(void);
void unregister_plugin_commands(const struct plugin_t *plugin);

#endif
