/*
 *   module.c -- May 18 2016 21:30:04 EDT
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
#include <foxbot/module.h>
#include <foxbot/list.h>

static dlink_list modules;

void
register_module(struct module_t *module)
{
    if (!module->register_func()) {
        do_error("Error loading module %s (%p)", module->name, module);
        return;
    }

    /* TODO: log system */
    printf("Registered plugin %s by %s, compiled %s.\n",
           module->name, module->author, module->build_time);

    dlink_insert(&modules, module);
}

void
load_module(const char *const name)
{
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s/%s", PLUIGIN_DIR, name);

    void *mod = dlopen(buf, RTLD_NOW);

    if (!mod) {
        do_error("Error opening %s: %s\n", name, dlerror());
        return;
    }

    void *obj = dlsym(mod, "_fox_module");
    register_module((struct module_t *) obj);
}

void
load_conf_modules(void)
{
    DLINK_FOREACH(node, dlist_head(&botconfig.conf_modules)) {
        const struct conf_multiple *const cm = dlink_data(node);
        if (cm->type == CONF_MODULE)
            load_module(cm->name);
    }

}
