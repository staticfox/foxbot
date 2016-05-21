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

#include <config.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include <foxbot/conf.h>
#include <foxbot/message.h>
#include <foxbot/parser.h>
#include <foxbot/plugin.h>
#include <foxbot/list.h>
#include <foxbot/user.h>

static dlink_list plugins;

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
        if ((fox_strcmp(temp->plugin->name, name) == 0) ||
            (fox_strcmp(temp->file_name, name) == 0)) {
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
        if ((fox_strcmp(temp->plugin->name, name) == 0) ||
            (fox_strcmp(temp->file_name, name) == 0)) {
            return true;
        }
    }

    return false;
}

static bool
is_valid_plugin(const struct plugin_handle_t *const p)
{
    if (p->plugin == NULL) {
        do_error("%s: No valid foxbot struct found.", p->file_name);
        return false;
    }

    if (p->plugin->name == NULL) {
        do_error("%s: Plugin name not found.", p->file_name);
        return false;
    }

    if (p->plugin->register_func == NULL) {
        do_error("%s: No registration function found.", p->file_name);
        return false;
    }

    if (p->plugin->unregister_func == NULL) {
        do_error("%s: No unregistration function found.", p->file_name);
        return false;
    }

    if (p->plugin->version == NULL) {
        do_error("%s: No version found.", p->file_name);
        return false;
    }

    if (p->plugin->description == NULL) {
        do_error("%s: No description found.", p->file_name);
        return false;
    }

    if (p->plugin->author == NULL) {
        do_error("%s: No author found.", p->file_name);
        return false;
    }

    if (p->plugin->build_time == NULL) {
        do_error("%s: No build_time found.", p->file_name);
        return false;
    }

    return true;
}

void
iregister_plugin(struct plugin_handle_t *plugin_handle, const bool announce)
{
    const struct plugin_t *const p = plugin_handle->plugin;

    if (!is_valid_plugin(plugin_handle)) {
        do_error("%s is not a valid FoxBot plugin.", plugin_handle->file_name);
        if (announce && bot.msg->from->nick) {
            notice(bot.msg->from->nick, "%s is not a valid FoxBot plugin.",
                   plugin_handle->file_name);
        }
        dlclose(plugin_handle->dlobj);
        xfree(plugin_handle->file_name);
        xfree(plugin_handle);
        return;
    }

    if (!p->register_func()) {
        do_error("Error loading plugin %s (%p)", p->name, plugin_handle->dlobj);
        if (announce && bot.msg->from->nick) {
            notice(bot.msg->from->nick, "Error loading plugin %s (%p)",
                   p->name, plugin_handle->dlobj);
        }
        dlclose(plugin_handle->dlobj);
        xfree(plugin_handle->file_name);
        xfree(plugin_handle);
        return;
    }

    /* TODO: log system */
    printf("Registered plugin %s by %s, compiled %s. Loaded at address %p.\n",
           p->name, p->author, p->build_time, plugin_handle->dlobj);

    if (announce && bot.msg->from->nick) {
        notice(bot.msg->from->nick, "Registered plugin %s by %s, compiled %s. Loaded at address %p.",
               p->name, p->author, p->build_time, plugin_handle->dlobj);
    }

    dlink_insert(&plugins, plugin_handle);
}

void
iunload_plugin(const char *const name, const bool announce)
{
    struct plugin_handle_t *plugin_handle = NULL;

    DLINK_FOREACH(node, dlist_head(&plugins)) {
        const struct plugin_handle_t *const temp = dlink_data(node);
        if ((fox_strcmp(temp->plugin->name, name) == 0) ||
            (fox_strcmp(temp->file_name, name) == 0)) {
            plugin_handle = dlink_data(node);
            dlink_delete(node, &plugins);
            break;
        }
    }

    if (plugin_handle == NULL) {
        do_error("Unable to find plugin %s.", name);
        if (announce && bot.msg->from->nick)
            notice(bot.msg->from->nick, "Unable to find plugin %s.", name);
        return;
    }

    const struct plugin_t *const p = plugin_handle->plugin;
    if (!p->unregister_func()) {
        if (announce && bot.msg->from->nick)
            notice(bot.msg->from->nick, "Error unloading plugin %s.", p->name);
        do_error("Error unloading plugin %s.", p->name);
        return;
    }

    int tmp = dlclose(plugin_handle->dlobj);
    if (tmp != 0) {
        if (announce && bot.msg->from->nick)
            notice(bot.msg->from->nick, "Error unloading plugin (dlclose): %s", dlerror());
        do_error("Error unloading plugin (dlclose): %s", dlerror());
        return;
    }

    xfree(plugin_handle->file_name);
    xfree(plugin_handle);
    if (announce && bot.msg->from->nick)
        notice(bot.msg->from->nick, "Unloaded plugin %s.", name);
}

void
iload_plugin(const char *const name, const bool announce)
{
    char buf[1024];
    static const struct plugin_handle_t EMPTY_PLUGIN_HANDLE;
    snprintf(buf, sizeof(buf), "%s/%s", PLUIGIN_DIR, name);

    void *mod = dlopen(buf, RTLD_NOW);

    if (!mod) {
        if (bot.msg->from->nick)
            notice(bot.msg->from->nick, "Error opening %s: %s\n", name, dlerror());
        do_error("Error opening %s: %s", name, dlerror());
        return;
    }

    void *obj = dlsym(mod, "fox_plugin");

    if (plugin_exists(((struct plugin_t *)obj)->name) || plugin_exists(name)) {
        if (bot.msg->from->nick)
            notice(bot.msg->from->nick, "%s is already loaded.", name);
        do_error("%s is already loaded.", name);
        dlclose(obj);
        return;
    }

    struct plugin_handle_t *plugin_handle = xmalloc(sizeof(*plugin_handle));
    *plugin_handle = EMPTY_PLUGIN_HANDLE;
    plugin_handle->dlobj = mod;
    plugin_handle->file_name = xstrdup(name);
    plugin_handle->plugin = (struct plugin_t *) obj;

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
