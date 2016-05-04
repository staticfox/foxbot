/*
 *   main.c -- May 1 2016 19:57:22 EST
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

#include <foxbot/config.h>

#include <stdio.h>
#include <dlfcn.h>

/* This was borrowed from PleXus4 */
int
main(int argc, char **argv)
{
    void *handle;
    union
    {
        int (*f)(int, char **);
        void *ptr;
    } u;

    handle = dlopen(LIBDIR "/" PACKAGE "/libfoxbot.so", RTLD_NOW | RTLD_GLOBAL);
    if (!handle)
    {
        fprintf(stderr, "Unable to open libfoxbot: %s\n", dlerror());
        return -1;
    }

    u.ptr = dlsym(handle, "main_foxbot");
    if (!u.ptr)
    {
        fprintf(stderr, "Unable to start libfoxbot: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }

    u.f(argc, argv);

    if (dlclose(handle))
    {
        fprintf(stderr, "Unable to close libfoxbot: %s\n", dlerror());
        return -1;
    }

    handle = dlopen(LIBDIR "/" PACKAGE "/libfoxbot.so", RTLD_NOW | RTLD_GLOBAL | RTLD_NOLOAD);
    if (handle)
    {
        fprintf(stderr, "Unable to unload libfoxbot, aborting\n");
        return -1;
    }
}
