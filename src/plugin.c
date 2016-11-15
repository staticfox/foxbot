/*
 *   plugin.c -- May 18 2016 21:30:04 EDT
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
#include <config.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <foxbot/admin.h>
#include <foxbot/conf.h>
#include <foxbot/message.h>
#include <foxbot/parser.h>
#include <foxbot/plugin.h>
#include <foxbot/list.h>
#include <foxbot/user.h>

static dlink_list plugins, commands;
static unsigned int longest_name = 0;

void
list_plugins(const char *const nick)
{
    notice(nick, "%d plugins loaded:", dlist_length(&plugins));
    DLINK_FOREACH(node, dlist_head(&plugins)) {
        const struct plugin_handle_t *const p = dlink_data(node);
        notice(nick, "%s - %s", p->plugin->name, p->file_name);
    }
}

struct plugin_handle_t *
get_plugin_info(const char *const name)
{
    DLINK_FOREACH(node, dlist_head(&plugins)) {
        struct plugin_handle_t *temp = dlink_data(node);
        if ((strcasecmp(temp->plugin->name, name) == 0) ||
            (strcasecmp(temp->file_name, name) == 0)) {
            return temp;
        }
    }

    return NULL;
}

static bool
plugin_exists(const char *const name)
{
    DLINK_FOREACH(node, dlist_head(&plugins)) {
        const struct plugin_handle_t *const temp = dlink_data(node);
        if ((strcasecmp(temp->plugin->name, name) == 0) ||
            (strcasecmp(temp->file_name, name) == 0)) {
            return true;
        }
    }

    return false;
}

static void
report_status(const bool announce, const char *const msg, ...)
{
    char buf[MAX_IRC_BUF] = { 0 };
    va_list ap;
    va_start(ap, msg);
    vsnprintf(buf, MAX_IRC_BUF, msg, ap);
    va_end(ap);

    do_error(buf);
    if (announce && bot.msg->from->nick)
        notice(bot.msg->from->nick, buf);
}

bool
is_valid_plugin(const char *const file, const struct plugin_t *const p, const bool announce)
{
    if (p == NULL) {
        report_status(announce, "%s: No valid foxbot struct found.", file);
        return false;
    }

    if (p->name == NULL) {
        report_status(announce, "%s: Plugin name not found.", file);
        return false;
    }

    if (p->register_func == NULL) {
        report_status(announce, "%s: No registration function found.", file);
        return false;
    }

    if (p->unregister_func == NULL) {
        report_status(announce, "%s: No unregistration function found.", file);
        return false;
    }

    if (p->version == NULL) {
        report_status(announce, "%s: No version found.", file);
        return false;
    }

    if (p->description == NULL) {
        report_status(announce, "%s: No description found.", file);
        return false;
    }

    if (p->author == NULL) {
        report_status(announce, "%s: No author found.", file);
        return false;
    }

    if (p->build_time == NULL) {
        report_status(announce, "%s: No build time found.", file);
        return false;
    }

    return true;
}

static void
dl_safe_close(lt_dlhandle addr)
{
    int res = lt_dlclose(addr);

    /* abort? */
    if (res != 0)
        report_status("Error closing shared library: %s", lt_dlerror());
}

static void
clean_up_plugin(struct plugin_handle_t *p)
{
    /* Until we can come up with a better way */
    if (!(bot.flags & RUNTIME_TEST))
        dl_safe_close(p->dlobj);
    xfree(p->file_name);
    xfree(p);
}

void
iregister_plugin(struct plugin_handle_t *plugin_handle, const bool announce)
{
    const struct plugin_t *const p = plugin_handle->plugin;

    if (!p->register_func()) {
        report_status(announce, "Error loading plugin %s (" PRIxPTR ")",
                      p->name, (uintptr_t)plugin_handle->dlobj);
        clean_up_plugin(plugin_handle);
        return;
    }

    report_status(announce, "Registered plugin %s by %s, compiled %s. "
                  "Loaded at address " PRIxPTR ".", p->name, p->author,
                  p->build_time, (uintptr_t)plugin_handle->dlobj);

    dlink_insert(&plugins, plugin_handle);
}

void
iunload_plugin(const char *const name, const bool announce)
{
    struct plugin_handle_t *plugin_handle = NULL;

    DLINK_FOREACH(node, dlist_head(&plugins)) {
        const struct plugin_handle_t *const temp = dlink_data(node);
        if ((strcasecmp(temp->plugin->name, name) == 0) ||
            (strcasecmp(temp->file_name, name) == 0)) {
            plugin_handle = dlink_data(node);
            dlink_delete(node, &plugins);
            break;
        }
    }

    if (plugin_handle == NULL) {
        report_status(announce, "Unable to find plugin %s.", name);
        return;
    }

    const struct plugin_t *const p = plugin_handle->plugin;
    if (!p->unregister_func()) {
        report_status(announce, "Error unloading plugin %s.", p->name);
        return;
    }

    unregister_plugin_commands(p);
    clean_up_plugin(plugin_handle);
    report_status(announce, "Unloaded plugin %s.", name);
}

void
iload_plugin(const char *const name, const bool announce)
{
    char buf[1024];
    static const struct plugin_handle_t EMPTY_PLUGIN_HANDLE;

    if (strstr(name, "../") || strstr(name, "/..")) {
        report_status(announce, "Plugin names cannot include pathes.");
        return;
    }

    if (strchr(name, '.') != NULL) {
        report_status(announce, "Plugin names cannot include dots.");
        return;
    }

    if (plugin_exists(name)) {
        report_status(announce, "%s is already loaded.", name);
        return;
    }

    const char *plugin_dir = getenv("FOXBOT_PLUGIN_DIR");
    plugin_dir = plugin_dir ? plugin_dir : PLUGIN_DIR;
    snprintf(buf, sizeof(buf), "%s/%s", plugin_dir, name);

    lt_dlhandle mod = lt_dlopenext(buf);

    if (!mod) {
        report_status(announce, "Error opening %s: %s", name, lt_dlerror());
        return;
    }

    void *obj = lt_dlsym(mod, "fox_plugin");
    struct plugin_t *plugin = (struct plugin_t *) obj;

    if (!is_valid_plugin(name, plugin, announce)) {
        report_status(announce, "%s is not a valid FoxBot plugin.", name);
        dl_safe_close(obj);
        return;
    }

    if (plugin_exists(plugin->name)) {
        report_status(announce, "%s is already loaded.", plugin->name);
        dl_safe_close(obj);
        return;
    }

    struct plugin_handle_t *plugin_handle = xmalloc(sizeof(*plugin_handle));
    *plugin_handle = EMPTY_PLUGIN_HANDLE;
    plugin_handle->dlobj = mod;
    plugin_handle->file_name = xstrdup(name);
    plugin_handle->plugin = plugin;

    iregister_plugin(plugin_handle, announce);
}

void
load_conf_plugins(void)
{
    DLINK_FOREACH(node, dlist_head(&botconfig.conf_modules)) {
        const struct conf_multiple *const cm = dlink_data(node);
        if (cm->type == CONF_PLUGIN)
            iload_plugin(cm->name, false);
    }
}

/* Commands */
static void
recheck_longest_name(void)
{
    struct command_t *command;
    longest_name = 0;

    DLINK_FOREACH(node, dlist_head(&commands)) {
        command = (struct command_t *) dlink_data(node);
        if (strlen(command->name) > longest_name)
            longest_name = strlen(command->name);
    }
}

void
register_command(const char *const command, const int params, const int access, const cmd_func func, struct plugin_t *p)
{
    if (find_command(false, command, true)) {
        do_error("You cannot register a command with duplicate names. [%s]", command);
        return;
    }

    static const struct command_t EMPTY_HOOK;
    struct command_t *hook = xmalloc(sizeof(*hook));
    *hook = EMPTY_HOOK;
    hook->name = xstrdup(command);
    hook->params = params;
    hook->access = access;
    hook->func = func;
    hook->plugin = p;
    dlink_insert(&commands, hook);

    if (strlen(hook->name) > longest_name)
        longest_name = strlen(hook->name);
}

void
unregister_command(const char *const command, const cmd_func func)
{
    struct command_t *hook = NULL;

    DLINK_FOREACH(node, dlist_head(&commands)) {
        hook = (struct command_t *) dlink_data(node);
        if (hook->func == func && (strcmp(hook->name, command) == 0)) {
            xfree(hook->name);
            xfree(hook);
            dlink_delete(node, &commands);
            return;
        }
    }

    do_error("Called unregister_command on a non-existant command! %s %p", command, func);
    recheck_longest_name();
}

void
unregister_plugin_commands(const struct plugin_t *const plugin)
{
    struct command_t *command;

    DLINK_FOREACH(node, dlist_head(&commands)) {
        command = (struct command_t *) dlink_data(node);
        if (command->plugin == plugin) {
            xfree(command->name);
            xfree(command);
            dlink_delete(node, &commands);
        }
    }

    recheck_longest_name();
}

static bool
access(int level)
{
    if (find_admin_access(bot.msg->from) > level)
        return true;

    if (!bot.msg->target_is_channel && bot.msg->from)
        notice(bot.msg->from->nick, "You are not authorized to preform this action.");

    return false;
}

void
exec_command(void)
{
    if (!bot.msg->params || !bot.msg->params[1]) return;
    const struct command_t *const command = find_command(true, bot.msg->params+1, false);
    if (command) {
        const char *param = strlen(bot.msg->params+1) - strlen(command->name) > 1 ?
            bot.msg->params + strlen(command->name) + 2 : NULL;
        command->func(param);
    }
}

static size_t
param_count(const char *const params)
{
    size_t num = 1, ii = 0;

    for (; params[ii]; ii++)
        if (params[ii] == ' ')
            num++;

    return num;
}

struct command_t *
find_command(const bool exec, const char *command, bool exact)
{
    struct command_t *hook, *tmp_hook = NULL;
    size_t match_size = 0;

    if (bot.msg->target_is_channel) {
        if (command[0] != botconfig.prefix)
            return NULL;
        ++command;
    }

    DLINK_FOREACH(node, dlist_head(&commands)) {
        hook = (struct command_t *) dlink_data(node);
        assert(hook->name);

        if (exact) {
            if (strcasecmp(hook->name, command) == 0) return hook;
        } else if (strncasecmp(hook->name, command, strlen(hook->name)) == 0) {
            if (strlen(hook->name) <= match_size) continue;
            match_size = strlen(hook->name);
            tmp_hook = hook;
        }
    }

    if (exact) return NULL;
    if (tmp_hook) {
        if (exec && !access(tmp_hook->access)) {
            return NULL;
        } else if (tmp_hook->params) {
            const char *param = strlen(bot.msg->params+1) - strlen(tmp_hook->name) > 1 ?
                bot.msg->params + strlen(tmp_hook->name) + 2 : NULL;
            if (param == NULL) {
                if (bot.msg->from)
                    notice(bot.msg->from->nick, "%s requires more parameters.", tmp_hook->name);
                return NULL;
            } else if (param_count(param) < tmp_hook->params) {
                if (bot.msg->from)
                    notice(bot.msg->from->nick, "%s requires more parameters.", tmp_hook->name);
                return NULL;
            }
        }

        return tmp_hook;
    }

    if (exec && bot.msg->from)
        notice(bot.msg->from->nick, "%s is an unknown command.", command);
    return NULL;
}

static void
show_help(const char *const params)
{
    (void) params;
    const int my_access = find_admin_access(bot.msg->from);
    notice(bot.msg->from->nick, "Your access level is %d", my_access);
    notice(bot.msg->from->nick, "Available commands:");

    DLINK_FOREACH(node, dlist_head(&commands)) {
        char buf[MAX_IRC_BUF];
        int ii = 0;
        const struct command_t *const p = dlink_data(node);

        /* Only show the user what they can run */
        if (my_access < p->access) continue;
        unsigned int length_needed = (longest_name - strlen(p->name)) + 3;
        ii += snprintf(buf+ii, sizeof(buf) - ii, "%s", p->name);
        if ((sizeof(buf) - ii) > length_needed) {
            memset(buf+ii, ' ', length_needed);
            snprintf(buf+ii+length_needed, sizeof(buf) - ii, "[%d]", p->access);
        }
        notice(bot.msg->from->nick, "%s", buf);
    }
}

void
register_default_commands(void)
{
    register_command("HELP", 0, 0, show_help, NULL);
}
