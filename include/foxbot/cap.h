/*
 *   cap.h -- May 7 2016 00:39:57 EST
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

#define ACCOUNTNOTIFY   0x00001
#define ACCOUNTTAG      0x00002
#define AWAYNOTIFY      0x00004
#define CAPNOTIFY       0x00008
#define CHGHOST         0x00010
#define ECHOMESSAGE     0x00020
#define EXTENDEDJOIN    0x00040
#define INVITENOTIFY    0x00080
#define MULTIPREFIX     0x00100
#define SASL            0x00200
#define SERVERTIME      0x00400
#define TLS             0x00800
#define USERHOSTINNAMES 0x01000

void handle_cap(void);
inline bool is_supported(int i) { return (bot.ircd->caps_supported & i); }
inline bool is_active(int i) { return (bot.ircd->caps_active & i); }

#endif
