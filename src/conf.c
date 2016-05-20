/*
 *   conf.c -- April 27 2016 19:45:28 EDT
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

#include <errno.h>
#include <string.h>

#include <foxbot/conf.h>
#include <foxbot/foxbot.h>
#include <foxbot/parser.h>

struct conf_parser_context conf_parser_ctx;
struct botconfig_entry botconfig;

extern int yyparse();

static void
clear_conf(void)
{
    static const struct botconfig_entry EMPTY_BOT;
    xfree(botconfig.nick);
    xfree(botconfig.ident);
    xfree(botconfig.host);
    xfree(botconfig.port);
    xfree(botconfig.realname);
    DLINK_FOREACH(node, dlist_head(&botconfig.conf_modules)) {
        struct conf_multiple *cm = dlink_data(node);
        xfree(cm->name);
        xfree(cm);
        dlink_delete(node, &botconfig.conf_modules);
    }
    /* in Epic Victory voice "EWWWWWWWWWW" */
    DLINK_FOREACH(node, dlist_head(&botconfig.admins)) {
        struct admin_struct_t *entry = dlink_data(node);
        DLINK_FOREACH(node2, dlist_head(&entry->ns_accts)) {
            char *data = dlink_data(node2);
            xfree(data);
            dlink_delete(node2, &entry->ns_accts);
        }
        DLINK_FOREACH(node2, dlist_head(&entry->hosts)) {
            char *data = dlink_data(node2);
            xfree(data);
            dlink_delete(node2, &entry->hosts);
        }
        xfree(entry);
        dlink_delete(node, &botconfig.admins);
    }
    botconfig = EMPTY_BOT;
}

static void
set_default_conf(void)
{
    clear_conf();
    botconfig.nick = xstrdup("foxbot");
    botconfig.ident = xstrdup("foxbot");
    botconfig.host = xstrdup("misconfigured.host");
    botconfig.port = xstrdup("-4");
    botconfig.realname = xstrdup(":3");
}

static void
override_test_port(void)
{
    if (bot.test_port > -1) {
        xfree(botconfig.port);
        char buf[65535];
        snprintf(buf, sizeof(buf), "%d", bot.test_port);
        botconfig.port = xstrdup(buf);
    }
}

static void
read_conf(FILE *file)
{
    set_default_conf();
    conf_parser_ctx.pass = 1;
    yyparse();
    rewind(file);
    conf_parser_ctx.pass = 2;
    clear_conf();
    yyparse(file);
    override_test_port();
}

void
add_m_safe(const char *const entry, const enum conf_multiple_types type)
{
    DLINK_FOREACH(node, dlist_head(&botconfig.conf_modules)) {
        struct conf_multiple *cm = dlink_data(node);
        if ((fox_strcmp(cm->name, entry) == 0) && cm->type == type)
            return;
    }

    static const struct conf_multiple CONF_EMPTY;
    struct conf_multiple *cm = xmalloc(sizeof(*cm));
    *cm = CONF_EMPTY;
    cm->name = xstrdup(entry);
    cm->type = type;
    dlink_insert(&botconfig.conf_modules, cm);
}

void
conf_set_ckey(const char *const entry,
              const enum conf_multiple_types type,
              const char *const key)
{
    DLINK_FOREACH(node, dlist_head(&botconfig.conf_modules)) {
        struct conf_multiple *cm = dlink_data(node);
        if ((fox_strcmp(cm->name, entry) == 0) && cm->type == type) {
            xfree(cm->key);
            cm->key = xstrdup(key);
        }
    }
}

struct admin_struct_t *
make_admin_conf(const char *const entry)
{
    static const struct admin_struct_t EMPTY_ADMIN;
    DLINK_FOREACH(node, dlist_head(&botconfig.admins)) {
        struct admin_struct_t *tmp = NULL;
        tmp = dlink_data(node);
        if (fox_strcmp(tmp->name, entry) == 0)
            return NULL;
    }
    struct admin_struct_t *admin = xmalloc(sizeof(*admin));
    *admin = EMPTY_ADMIN;
    admin->name = xstrdup(entry);
    dlink_insert(&botconfig.admins, admin);
    return admin;
}

void
admin_add_data(struct admin_struct_t *entry,
               const enum admin_data_flag type,
               const char *const data)
{
    if (type == CONF_ADMIN_NS) {
        DLINK_FOREACH(node, dlist_head(&entry->ns_accts)) {
            if (fox_strcmp(dlink_data(node), data) == 0) {
                return;
            }
        }
        dlink_insert(&entry->ns_accts, xstrdup(data));
    } else if (type == CONF_ADMIN_HOST) {
        DLINK_FOREACH(node, dlist_head(&entry->hosts)) {
            if (fox_strcmp(dlink_data(node), data) == 0) {
                return;
            }
        }
        dlink_insert(&entry->hosts, xstrdup(data));
    }
}

void
read_conf_file(void)
{
    const char *filename;

    if (conf_parser_ctx.config_file_path != NULL)
        filename = conf_parser_ctx.config_file_path;
    else
        filename = SYSCONFDIR "/foxbot.conf";

    if ((conf_parser_ctx.conf_file = fopen(filename, "r")) == NULL) {
        do_error("Unable to read %s: %s", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    read_conf(conf_parser_ctx.conf_file);
    fclose(conf_parser_ctx.conf_file);
}

void
yyerror(const char *message)
{
    if (conf_parser_ctx.pass != 1)
        return;

    do_error((char *) message);
    exit(EXIT_FAILURE);
}
