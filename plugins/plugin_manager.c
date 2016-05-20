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

#include <foxbot/admin.h>
#include <foxbot/foxbot.h>
#include <foxbot/hook.h>
#include <foxbot/memory.h>
#include <foxbot/message.h>
#include <foxbot/parser.h>
#include <foxbot/plugin.h>
#include <foxbot/user.h>

struct plugin_t fox_plugin;

static bool
access(void)
{
    if (find_admin_access(bot.msg->from) > 900)
        return true;

    if (!bot.msg->target_is_channel)
        notice(bot.msg->from->nick, "You are not authorized to preform this action.");

    return false;
}

static bool
has_params(const char *const phrase,
                  const char *const used,
                  const char *const sub)
{
    if (phrase == NULL || strlen(phrase) < 1 || phrase[0] == '\0' || isspace(phrase[0])) {
        notice(bot.msg->from->nick, "%s %s requires more parameters.", used, sub);
        return false;
    }

    return true;
}

static bool
is_protected_plugin(const char *const name)
{
    if (fox_strcmp(name, fox_plugin.name) == 0) {
        notice(bot.msg->from->nick, "%s cannot be modified.", name);
        return true;
    }

    return false;
}

static void
plugin_manage_command(void)
{
    char *command = NULL;
    const char *cmd_used = NULL;
    static const char trigger = '.';
    static const char *const plugin_cmd  = "PLUGIN";
    static const char *const plugins_cmd = "PLUGINS";
    static const char *const load_cmd    = "LOAD";
    static const char *const unload_cmd  = "UNLOAD";
    static const char *const reload_cmd  = "RELOAD";
    static const char *const info_cmd    = "INFO";
    static const char *const list_cmd    = "LIST";
    static const char *const help_cmd    = "HELP";

    char *tofree, *message;
    tofree = message = xstrdup(bot.msg->params+1);

    /* Check if we received a private message or an inchannel trigger. */
    if (message[0] == trigger && !bot.msg->target_is_channel)
        goto done;

    command = fox_strsep(&message, " ");

    if (bot.msg->target_is_channel)
        memmove(command, command+1, strlen(command));

    if (fox_strcmp(command, plugin_cmd) == 0)
        cmd_used = plugin_cmd;
    else if (fox_strcmp(command, plugins_cmd) == 0)
        cmd_used = plugins_cmd;
    else
        goto done;

    if (!access())
        goto done;

    /* Sub-command */
    if (!(command = fox_strsep(&message, " "))) {
        notice(bot.msg->from->nick, "%s requires more parameters.", cmd_used);
        goto done;
    }

    /* Full phrase (if it exists) */
    const char *phrase = NULL;
    const size_t param_length = strlen(bot.msg->params);
    if (bot.msg->target_is_channel) {
        size_t len = 4 + strlen(cmd_used) + strlen(command);
        if (len < param_length)
            phrase = bot.msg->params + len;
    } else {
        size_t len = 3 + strlen(cmd_used) + strlen(command);
        if (len < param_length)
            phrase = bot.msg->params + len;
    }

    /* Parse sub-commands
     * TODO: CLEAN ME
     */
    if (fox_strcmp(command, load_cmd) == 0) {
        /* plugin load */
        if (!has_params(phrase, cmd_used, load_cmd))
            goto done;
        if (is_protected_plugin(phrase))
            goto done;
        notice(bot.msg->from->nick, "Loading %s.", phrase);
        iload_plugin(phrase, true);
    } else if (fox_strcmp(command, unload_cmd) == 0) {
        /* plugin unload */
        if (!has_params(phrase, cmd_used, unload_cmd))
            goto done;
        if (is_protected_plugin(phrase))
            goto done;
        notice(bot.msg->from->nick, "Unloading %s.", phrase);
        iunload_plugin(phrase, true);
    } else if (fox_strcmp(command, reload_cmd) == 0) {
        /* plugin reload */
        if (!has_params(phrase, cmd_used, reload_cmd))
            goto done;
        if (is_protected_plugin(phrase))
            goto done;
        notice(bot.msg->from->nick, "Reloading %s.", phrase);
        iunload_plugin(phrase, true);
        iload_plugin(phrase, true);
    } else if (fox_strcmp(command, info_cmd) == 0) {
        /* plugin info */
        if (!has_params(phrase, cmd_used, info_cmd))
            goto done;
        const struct plugin_handle_t *const plugin_info = get_plugin_info(phrase);
        if (!plugin_info) {
            notice(bot.msg->from->nick, "%s is not loaded.", phrase);
            goto done;
        }
        notice(bot.msg->from->nick, "Information on %s:", phrase);
        notice(bot.msg->from->nick, " ");
        notice(bot.msg->from->nick, "File: %s", plugin_info->file_name);
        notice(bot.msg->from->nick, "Name: %s", plugin_info->plugin->name);
        notice(bot.msg->from->nick, "Description: %s", plugin_info->plugin->description);
        notice(bot.msg->from->nick, "Version: %s", plugin_info->plugin->version);
        notice(bot.msg->from->nick, "Author: %s", plugin_info->plugin->author);
        notice(bot.msg->from->nick, "Compiled: %s", plugin_info->plugin->build_time);
        notice(bot.msg->from->nick, "Loaded at %p", plugin_info->dlobj);
    } else if (fox_strcmp(command, list_cmd) == 0) {
        /* plugin list */
        list_plugins(bot.msg->from->nick);
    } else if (fox_strcmp(command, help_cmd) == 0) {
        /* plugin help. TODO: help sub-command */
        notice(bot.msg->from->nick, "HELP for %s: ", cmd_used);
        notice(bot.msg->from->nick, "%s allows you to manage FoxBot's dynamic plugin system.", cmd_used);
        notice(bot.msg->from->nick, "Available sub-commands are: LOAD, UNLOAD, RELOAD, INFO, LIST, HELP");
    } else {
        /* plugin ??? */
        notice(bot.msg->from->nick, "%s %s is an unknown sub-command.", cmd_used, command);
    }

done:
    xfree(tofree);
}

/* Plugin manager call backs */
static bool
register_plugin(void)
{
    add_hook("on_privmsg", (hook_func) plugin_manage_command);
    return true;
}

static bool
unregister_plugin(void)
{
    delete_hook("on_privmsg", (hook_func) plugin_manage_command);
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
