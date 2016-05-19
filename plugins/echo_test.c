/*
 *   echo_test.c -- May 18 2016 20:16:39 EDT
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

#include <stdio.h>

#include <foxbot/foxbot.h>
#include <foxbot/hook.h>
#include <foxbot/message.h>
#include <foxbot/module.h>
#include <foxbot/user.h>

void
say_my_name(void)
{
    char buf[MAX_IRC_BUF];
    snprintf(buf, sizeof(buf), "Hi %s! My name is %s!", bot.msg->from->nick, bot.user->nick);
    privmsg(bot.msg->target, buf);
}

bool
register_plugin(void)
{
    add_hook("on_privmsg", (hook_func) say_my_name);
    return true;
}

bool
unregister_plugin(void)
{
    delete_hook("on_privmsg", (hook_func) say_my_name);
    return true;
}

struct module_t _fox_module = {
    .name = "Echo Test",                   /* Name of the plugin */
    .register_func = &register_plugin,     /* Register callback function */
    .unregister_func = &unregister_plugin, /* Unregister callback function */
    .version = "0.0.1",                    /* Plugin version */
    .description = "A simple test plugin", /* Plugin description */
    .author = "staticfox",                 /* Author of plugin */
    .build_time = __DATE__", "__TIME__,    /* Plugin compilation time */
};
