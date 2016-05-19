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
#include <foxbot/plugin.h>
#include <foxbot/list.h>

static dlink_list plugins;

void
iregister_plugin(struct plugin_t *plugin)
{
    if (!plugin->register_func()) {
        do_error("Error loading plugin %s (%p)", plugin->name, plugin);
        return;
    }

    /* TODO: log system */
    printf("Registered plugin %s by %s, compiled %s.\n",
           plugin->name, plugin->author, plugin->build_time);

    dlink_insert(&plugins, plugin);
}

void
iload_plugin(const char *const name)
{
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s/%s", PLUIGIN_DIR, name);

    void *mod = dlopen(buf, RTLD_NOW);

    if (!mod) {
        do_error("Error opening %s: %s\n", name, dlerror());
        return;
    }

    void *obj = dlsym(mod, "fox_plugin");
    iregister_plugin((struct plugin_t *) obj);
}

void
load_conf_plugins(void)
{
    DLINK_FOREACH(node, dlist_head(&botconfig.conf_modules)) {
        const struct conf_multiple *const cm = dlink_data(node);
        if (cm->type == CONF_PLUGIN)
            iload_plugin(cm->name);
    }
}
