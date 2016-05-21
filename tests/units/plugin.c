/*
 *   plugin.c -- May 21 2016 00:02:35 EDT
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

#include <foxbot/foxbot.h>
#include <foxbot/memory.h>
#include <foxbot/message.h>
#include <foxbot/plugin.h>
#include <foxbot/user.h>

#include "../check_foxbot.h"
#include "../check_server.h"

static bool
test_reg(void)
{
    return true;
}

static bool
test_unreg(void)
{
    return true;
}

static bool
test_reg_fail(void)
{
    return false;
}

static bool
test_unreg_fail(void)
{
    return false;
}

static const struct plugin_handle_t EMPTY_PLUGIN_HANDLE;
static const struct plugin_t EMPTY_PLUGIN;

START_TEST(check_load_plugin_loader)
{
    begin_test();

    bot.msg->from = bot.user;
    iload_plugin("plugin_manager.so", true);
    yield_to_server();

    ck_assert_ptr_ne(strstr(last_buffer, "NOTICE foxbot :Registered plugin Plugin Manager by staticfox"), NULL);

    end_test();
}
END_TEST

START_TEST(check_load_plugin_path)
{
    begin_test();

    bot.msg->from = bot.user;

    iload_plugin("../rogue.so", true);
    wait_for_last_buf("NOTICE foxbot :Plugin names cannot include pathes.");

    iload_plugin("plugin_manager.so", true);
    end_test();
}
END_TEST

START_TEST(check_load_plugin_exists)
{
    begin_test();

    bot.msg->from = bot.user;

    iload_plugin("echo_test.so", true);
    wait_for_last_buf("NOTICE foxbot :echo_test.so is already loaded.");

    end_test();
}
END_TEST

START_TEST(check_load_plugin_unknown)
{
    begin_test();

    bot.msg->from = bot.user;

    iload_plugin("unknownplugin.so", true);
    yield_to_server();
    ck_assert_ptr_ne(strstr(last_buffer, "NOTICE foxbot :Error opening unknownplugin.so: "), NULL);

    end_test();
}
END_TEST

START_TEST(check_load_plugin_nullptr)
{
    begin_test();

    bot.msg->from = bot.user;

    ck_assert_int_eq(is_valid_plugin("unknown.so", NULL, true), 0);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :unknown.so: No valid foxbot struct found.");

    end_test();
}
END_TEST

START_TEST(check_load_plugin_noname)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin;
    fox_plugin = EMPTY_PLUGIN;

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 0);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :unknown.so: Plugin name not found.");

    end_test();
}
END_TEST

START_TEST(check_load_plugin_noregfunc)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin;
    fox_plugin = EMPTY_PLUGIN;
    fox_plugin.name = xstrdup("Unit Test");

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 0);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :unknown.so: No registration function found.");

    xfree((char *) fox_plugin.name);

    end_test();
}
END_TEST

START_TEST(check_load_plugin_nounregfunc)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin;
    fox_plugin = EMPTY_PLUGIN;
    fox_plugin.name = xstrdup("Unit Test");
    fox_plugin.register_func = &test_reg;

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 0);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :unknown.so: No unregistration function found.");
    xfree((char *) fox_plugin.name);

    end_test();
}
END_TEST

START_TEST(check_load_plugin_noversion)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin;
    fox_plugin = EMPTY_PLUGIN;
    fox_plugin.name = xstrdup("Unit Test");
    fox_plugin.register_func = &test_reg;
    fox_plugin.unregister_func = &test_unreg;

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 0);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :unknown.so: No version found.");
    xfree((char *) fox_plugin.name);

    end_test();
}
END_TEST

START_TEST(check_load_plugin_nodescription)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin;
    fox_plugin = EMPTY_PLUGIN;
    fox_plugin.name = xstrdup("Unit Test");
    fox_plugin.register_func = &test_reg;
    fox_plugin.unregister_func = &test_unreg;
    fox_plugin.version = xstrdup("0.0.0");

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 0);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :unknown.so: No description found.");
    xfree((char *) fox_plugin.name);
    xfree((char *) fox_plugin.version);

    end_test();
}
END_TEST

START_TEST(check_load_plugin_noauthor)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin;
    fox_plugin = EMPTY_PLUGIN;
    fox_plugin.name = xstrdup("Unit Test");
    fox_plugin.register_func = &test_reg;
    fox_plugin.unregister_func = &test_unreg;
    fox_plugin.version = xstrdup("0.0.0");
    fox_plugin.description = xstrdup("Some description");

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 0);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :unknown.so: No author found.");
    xfree((char *) fox_plugin.name);
    xfree((char *) fox_plugin.version);
    xfree((char *) fox_plugin.description);

    end_test();
}
END_TEST

START_TEST(check_load_plugin_nobuildtime)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin;
    fox_plugin = EMPTY_PLUGIN;
    fox_plugin.name = xstrdup("Unit Test");
    fox_plugin.register_func = &test_reg;
    fox_plugin.unregister_func = &test_unreg;
    fox_plugin.version = xstrdup("0.0.0");
    fox_plugin.description = xstrdup("Some description");
    fox_plugin.author = xstrdup("staticfox");

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 0);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :unknown.so: No build time found.");
    xfree((char *) fox_plugin.name);
    xfree((char *) fox_plugin.version);
    xfree((char *) fox_plugin.description);
    xfree((char *) fox_plugin.author);

    end_test();
}
END_TEST

START_TEST(check_load_plugin_valid)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin = {
        .name = "Unit Test",
        .register_func = &test_reg,
        .unregister_func = &test_unreg,
        .version = "0.0.0",
        .description = "Some description",
        .author = "staticfox",
        .build_time = __DATE__", "__TIME__,
    };

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 1);

    end_test();
}
END_TEST

START_TEST(check_load_plugin_regfailure)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin = {
        .name = "Unit Test",
        .register_func = &test_reg_fail,
        .unregister_func = &test_unreg,
        .version = "0.0.0",
        .description = "Some description",
        .author = "staticfox",
        .build_time = __DATE__", "__TIME__,
    };

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 1);

    /* iregister_plugin automatically frees this */
    struct plugin_handle_t *plugin_handle = xmalloc(sizeof(*plugin_handle));
    *plugin_handle = EMPTY_PLUGIN_HANDLE;
    plugin_handle->dlobj = NULL;
    plugin_handle->plugin = &fox_plugin;

    iregister_plugin(plugin_handle, true);
    yield_to_server();

    ck_assert_ptr_ne(strstr(last_buffer, "NOTICE foxbot :Error loading plugin"), NULL);

    end_test();
}
END_TEST

START_TEST(check_load_plugin_regsuccess)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin = {
        .name = "Unit Test",
        .register_func = &test_reg,
        .unregister_func = &test_unreg,
        .version = "0.0.0",
        .description = "Some description",
        .author = "staticfox",
        .build_time = __DATE__", "__TIME__,
    };

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 1);

    struct plugin_handle_t *plugin_handle = xmalloc(sizeof(*plugin_handle));
    *plugin_handle = EMPTY_PLUGIN_HANDLE;
    plugin_handle->file_name = xstrdup("unknown.so");
    plugin_handle->dlobj = NULL;
    plugin_handle->plugin = &fox_plugin;

    iregister_plugin(plugin_handle, true);
    yield_to_server();

    ck_assert_ptr_ne(strstr(last_buffer, "NOTICE foxbot :Registered plugin Unit Test by staticfox"), NULL);

    xfree(plugin_handle->file_name);
    xfree(plugin_handle);

    end_test();
}
END_TEST

START_TEST(check_plugin_info)
{
    begin_test();

    ck_assert_ptr_ne(get_plugin_info("echo_test.so"), NULL);
    ck_assert_ptr_eq(get_plugin_info("unknown.so"), NULL);

    end_test();
}
END_TEST

START_TEST(check_plugin_list)
{
    begin_test();

    list_plugins("foxbot");

    wait_for_last_buf("NOTICE foxbot :Echo Test - echo_test.so");

    end_test();
}
END_TEST

START_TEST(check_unload_plugin_notexists)
{
    begin_test();

    bot.msg->from = bot.user;

    iunload_plugin("unknown.so", true);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :Unable to find plugin unknown.so.");

    end_test();
}
END_TEST

START_TEST(check_unload_plugin_unregfailure)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin = {
        .name = "Unit Test",
        .register_func = &test_reg,
        .unregister_func = &test_unreg_fail,
        .version = "0.0.0",
        .description = "Some description",
        .author = "staticfox",
        .build_time = __DATE__", "__TIME__,
    };

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 1);

    struct plugin_handle_t *plugin_handle = xmalloc(sizeof(*plugin_handle));
    *plugin_handle = EMPTY_PLUGIN_HANDLE;
    plugin_handle->file_name = xstrdup("unknown.so");
    plugin_handle->dlobj = NULL;
    plugin_handle->plugin = &fox_plugin;

    iregister_plugin(plugin_handle, true);
    yield_to_server();

    ck_assert_ptr_ne(strstr(last_buffer, "NOTICE foxbot :Registered plugin Unit Test by staticfox"), NULL);

    iunload_plugin("unknown.so", true);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :Error unloading plugin Unit Test.");

    xfree(plugin_handle->file_name);
    xfree(plugin_handle);

    end_test();
}
END_TEST

START_TEST(check_unload_plugin_unregsuccess)
{
    begin_test();

    bot.msg->from = bot.user;

    struct plugin_t fox_plugin = {
        .name = "Unit Test",
        .register_func = &test_reg,
        .unregister_func = &test_unreg,
        .version = "0.0.0",
        .description = "Some description",
        .author = "staticfox",
        .build_time = __DATE__", "__TIME__,
    };

    ck_assert_int_eq(is_valid_plugin("unknown.so", &fox_plugin, true), 1);

    /* automatically gets freed in iunload_plugin */
    struct plugin_handle_t *plugin_handle = xmalloc(sizeof(*plugin_handle));
    *plugin_handle = EMPTY_PLUGIN_HANDLE;
    plugin_handle->file_name = xstrdup("unknown.so");
    plugin_handle->dlobj = NULL;
    plugin_handle->plugin = &fox_plugin;

    iregister_plugin(plugin_handle, true);
    yield_to_server();

    ck_assert_ptr_ne(strstr(last_buffer, "NOTICE foxbot :Registered plugin Unit Test by staticfox"), NULL);

    iunload_plugin("unknown.so", true);
    yield_to_server();
    ck_assert_str_eq(last_buffer, "NOTICE foxbot :Unloaded plugin unknown.so.");

    end_test();
}
END_TEST

void
plugin_setup(Suite *s)
{
    TCase *tc = tcase_create("plugin");

    tcase_add_checked_fixture(tc, NULL, delete_foxbot);
    tcase_add_test(tc, check_load_plugin_loader);
    tcase_add_test(tc, check_load_plugin_path);
    tcase_add_test(tc, check_load_plugin_exists);
    tcase_add_test(tc, check_load_plugin_unknown);
    tcase_add_test(tc, check_load_plugin_nullptr);
    tcase_add_test(tc, check_load_plugin_noname);
    tcase_add_test(tc, check_load_plugin_noregfunc);
    tcase_add_test(tc, check_load_plugin_nounregfunc);
    tcase_add_test(tc, check_load_plugin_noversion);
    tcase_add_test(tc, check_load_plugin_nodescription);
    tcase_add_test(tc, check_load_plugin_noauthor);
    tcase_add_test(tc, check_load_plugin_nobuildtime);
    tcase_add_test(tc, check_load_plugin_valid);
    tcase_add_test(tc, check_load_plugin_regfailure);
    tcase_add_test(tc, check_load_plugin_regsuccess);
    tcase_add_test(tc, check_plugin_info);
    tcase_add_test(tc, check_plugin_list);
    tcase_add_test(tc, check_unload_plugin_notexists);
    tcase_add_test(tc, check_unload_plugin_unregfailure);
    tcase_add_test(tc, check_unload_plugin_unregsuccess);

    suite_add_tcase(s, tc);
}
