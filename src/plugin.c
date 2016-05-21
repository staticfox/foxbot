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
#include <stdarg.h>
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
dl_safe_close(void *addr)
{
    int res = dlclose(addr);

    /* abort? */
    if (res != 0)
        report_status("Error closing shared library: %s", dlerror());
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
        report_status(announce, "Error loading plugin %s (%p)", p->name, plugin_handle->dlobj);
        clean_up_plugin(plugin_handle);
        return;
    }

    report_status(announce, "Registered plugin %s by %s, compiled %s. Loaded at address %p.",
           p->name, p->author, p->build_time, plugin_handle->dlobj);

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
        report_status(announce, "Unable to find plugin %s.", name);
        return;
    }

    const struct plugin_t *const p = plugin_handle->plugin;
    if (!p->unregister_func()) {
        report_status(announce, "Error unloading plugin %s.", p->name);
        return;
    }

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

    if (plugin_exists(name)) {
        report_status(announce, "%s is already loaded.", name);
        return;
    }

    snprintf(buf, sizeof(buf), "%s/%s", PLUIGIN_DIR, name);

    void *mod = dlopen(buf, RTLD_NOW);

    if (!mod) {
        report_status(announce, "Error opening %s: %s", name, dlerror());
        return;
    }

    void *obj = dlsym(mod, "fox_plugin");
    struct plugin_t *plugin = (struct plugin_t *) obj;

    if (!is_valid_plugin(name, plugin, announce)) {
        report_status(announce, "%s is not a valid FoxBot plugin.", name);
        dl_safe_close(obj);
        return;
    }

    if (plugin_exists(plugin->name)) {
        report_status(announce, "%s is already loaded.", name);
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
