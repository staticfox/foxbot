/*
 *   signal.c -- April 27 2016 15:02:57 EST
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
#include <string.h> /* for memset */

#include "foxbot.h"
#include "signal.h"

static void
sigint_handler(int sig)
{
    (void)sig;
    quitting = 2;
}

void
setup_signals(void)
{
    sigset_t sigs;
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    sigemptyset(&sigs);

    act.sa_handler = sigint_handler;
    sigaddset(&act.sa_mask, SIGINT);
    sigaction(SIGINT, &act, 0);
    sigaddset(&sigs, SIGINT);
}
