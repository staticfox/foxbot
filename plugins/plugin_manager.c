/*
 *   plugin_manager.c -- May 20 2016 03:45:50 EDT
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

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <foxbot/admin.h>
#include <foxbot/conf.h>
#include <foxbot/foxbot.h>
#include <foxbot/hook.h>
#include <foxbot/memory.h>
#include <foxbot/message.h>
#include <foxbot/parser.h>
#include <foxbot/plugin.h>
#include <foxbot/user.h>
#include <foxbot/utility.h>

struct plugin_t fox_plugin;

static bool
is_protected_plugin(const char *const name)
{
    if (strcasecmp(name, fox_plugin.name) == 0) {
        notice(bot.msg->from->nick, "%s cannot be modified.", name);
        return true;
    }

    return false;
}

static bool
is_valid_plugin_name(const char *const name)
{
    if (strstr(name, " ")) {
        notice(bot.msg->from->nick, "Plugin names cannot have spaces.");
        return false;
    }

    if (strstr(name, "../") || strstr(name, "/..")) {
        notice(bot.msg->from->nick, "Plugin names cannot include pathes.");
        return false;
    }

    return true;
}

static void
command_plugin_list(const char *const params)
{
    (void) params;
    list_plugins(bot.msg->from->nick);
}

static void
command_plugin_help(const char *const params)
{
    (void) params;
    notice(bot.msg->from->nick, "HELP for PLUGIN:");
    notice(bot.msg->from->nick, "PLUGIN allows you to manage FoxBot's dynamic plugin system.");
    notice(bot.msg->from->nick, "Available sub-commands are: LOAD, UNLOAD, RELOAD, INFO, LIST, HELP");
}

static void
command_plugin_load(const char *const params)
{
    if (!is_valid_plugin_name(params)) return;
    if (is_protected_plugin(params)) return;
    notice(bot.msg->from->nick, "Loading %s.", params);
    iload_plugin(params, true);
}

static void
command_plugin_unload(const char *const params)
{
    if (!is_valid_plugin_name(params)) return;
    if (is_protected_plugin(params)) return;
    notice(bot.msg->from->nick, "Unloading %s.", params);
    iunload_plugin(params, true);
}

static void
command_plugin_reload(const char *const params)
{
    if (!is_valid_plugin_name(params)) return;
    if (is_protected_plugin(params)) return;
    notice(bot.msg->from->nick, "Reloading %s.", params);
    iunload_plugin(params, true);
    iload_plugin(params, true);
}

static void
command_plugin_info(const char *const params)
{
    if (!is_valid_plugin_name(params)) return;
    const struct plugin_handle_t *const plugin_info = get_plugin_info(params);
    if (!plugin_info) {
        notice(bot.msg->from->nick, "%s is not loaded.", params);
        return;
    }
    notice(bot.msg->from->nick, "Information on %s:", params);
    notice(bot.msg->from->nick, " ");
    notice(bot.msg->from->nick, "File: %s", plugin_info->file_name);
    notice(bot.msg->from->nick, "Name: %s", plugin_info->plugin->name);
    notice(bot.msg->from->nick, "Description: %s", plugin_info->plugin->description);
    notice(bot.msg->from->nick, "Version: %s", plugin_info->plugin->version);
    notice(bot.msg->from->nick, "Author: %s", plugin_info->plugin->author);
    notice(bot.msg->from->nick, "Compiled: %s", plugin_info->plugin->build_time);
    notice(bot.msg->from->nick, "Loaded at %p", plugin_info->dlobj);
}

static void
command_plugin_need_more_params(const char *const params)
{
    (void) params;
    notice(bot.msg->from->nick, "PLUGIN requires more parameters.");
}

/* Plugin manager call backs */
static bool
register_plugin(void)
{
    REG("PLUGIN", 0, 900, command_plugin_need_more_params);
    REG("PLUGIN LIST", 0, 900, command_plugin_list);
    REG("PLUGIN HELP", 0, 900, command_plugin_help);
    REG("PLUGIN LOAD", 1, 900, command_plugin_load);
    REG("PLUGIN UNLOAD", 1, 900, command_plugin_unload);
    REG("PLUGIN RELOAD", 1, 900, command_plugin_reload);
    REG("PLUGIN INFO", 1, 900, command_plugin_info);
    REG("PLUGINS", 0, 900, command_plugin_need_more_params);
    REG("PLUGINS LIST", 0, 900, command_plugin_list);
    REG("PLUGINS HELP", 0, 900, command_plugin_help);
    REG("PLUGINS LOAD", 1, 900, command_plugin_load);
    REG("PLUGINS UNLOAD", 1, 900, command_plugin_unload);
    REG("PLUGINS RELOAD", 1, 900, command_plugin_reload);
    REG("PLUGINS INFO", 1, 900, command_plugin_info);
    return true;
}

static bool
unregister_plugin(void)
{
    return true;
}

struct plugin_t fox_plugin = {
    .name = "Plugin Manager",
    .register_func = &register_plugin,
    .unregister_func = &unregister_plugin,
    .version = "0.0.1",
    .description = "FoxBot Core Plugin Manager",
    .author = "staticfox",
    .build_time = __DATE__", "__TIME__,
};
