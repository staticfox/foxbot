/*
 *   foxsignal.c -- April 27 2016 15:02:57 EST
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

#define _POSIX_C_SOURCE 201112L

#include <stdio.h>

#include "foxbot.h"
#include "foxsignal.h"

static void
sigusr1_handler(int sig)
{
    (void) sig;
    char buf[MAX_IRC_BUF];
    snprintf(buf, sizeof(buf), "%s", "Out of memory!");
    do_error(buf);

    if (is_registered())
        do_quit(buf);
}

static void
sigint_handler(int sig)
{
    char buf[MAX_IRC_BUF];

    if (is_registered()) {
        snprintf(buf, sizeof(buf), "Exiting on signal %d", sig);
        do_quit(buf);
    }

    quitting = true;
}

void
setup_signals(void)
{
    sigset_t sigs;
    struct sigaction act;

    sigemptyset(&sigs);

    act.sa_handler = sigusr1_handler;
    sigaddset(&act.sa_mask, SIGUSR1);
    sigaction(SIGUSR1, &act, 0);
    sigaddset(&sigs, SIGUSR1);

    act.sa_handler = sigint_handler;
    sigaddset(&act.sa_mask, SIGINT);
    sigaction(SIGINT, &act, 0);
    sigaddset(&sigs, SIGINT);
}
