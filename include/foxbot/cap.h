/*
 *   cap.h -- May 7 2016 00:39:57 EDT
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

#ifndef FOX_CAP_H_
#define FOX_CAP_H_

#include <stdbool.h>

#include <foxbot/ircd.h>
#include <foxbot/foxbot.h>

#define CAP_OPTS sizeof(capabilities)/sizeof(*capabilities)

#define ACCOUNTNOTIFY   0x000001
#define ACCOUNTTAG      0x000002
#define AWAYNOTIFY      0x000004
#define CAPNOTIFY       0x000008
#define CHGHOST         0x000010
#define ECHOMESSAGE     0x000020
#define EXTENDEDJOIN    0x000040
#define INVITENOTIFY    0x000080
#define MULTIPREFIX     0x000100
#define SASL            0x000200
#define SERVERTIME      0x000400
#define TLS             0x000800
#define USERHOSTINNAMES 0x001000
#define FOXBOTUNITTEST  0x100000

void handle_cap(void);
bool is_sticky(unsigned int cap);
static inline bool cap_supported(int i) { return (bot.ircd->caps_supported & i); }
static inline bool cap_active(int i) { return (bot.ircd->caps_active & i); }

#endif
