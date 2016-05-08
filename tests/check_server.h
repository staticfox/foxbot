/*
 *   check_server.h -- May 1 2016 9:39:18 EDT
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

/*

   Here's how the test system currently works:

   There are three threads:

   - Main thread: this is where the tests are being run.  The bot is run on
     this thread.

   - Server read thread (a.k.a. start_listener [1]): this thread accepts the
     connection, spawns the write thread, and then blockingly reads data
     coming from the bot.

   - Server write thread: this thread reads the write_queue and sends the
     data queued in it to foxbot.

   [1]: Naming convention in these test modules is really haphazard...
        Should probably be fixed some day.

   Note that there are 3 things we are synchronizing here: the two server
   threads as well as the bot's "pseudothread".  We say "pseudothread" because
   it's actually run on the main thread *on demand* (through yield_to_foxbot).

   Keep in mind that fox_write writes *asynchronously*: the data won't be
   written until the server write thread gets around to it.  So if you want
   make sure the data is actually written, you must call one of the
   yield_to_X functions.

   - yield_to_bot: allow the bot to process all pending messages.  It won't
     return until all of them have been processed by bot.

   - yield_to_server: wait until the server has processed all messages up to
     this point.  This indirectly triggers a yield_to_bot as well. [2]

   Generally speaking, a yield_to_server is a stronger synchronization
   barrier.  However, sometimes even yield_to_server is not enough.  For
   example, if the server performs fox_write while processing a message,
   there's no guarantee that the fox_write will go through yet.  So in this
   case you have to actually do yield_to_server followed by yield_to_bot to
   make sure the bot has received that message.

   I believe it should be safe to have more yields than is necessary, but too
   few yields can cause a test to stall forever.  When in doubt, add more
   yields ... X3

   Implementation details:

   Both of these features require a special message used for synchronization
   purposes, which we call the "pause" message.  It's basically a single line
   starting with '#' sent to the bot.  If the line also contains some other
   characters after the '#', the bot will forward them to the server.  Hence,
   a message of "##" will cause the server to receive "#".  This is how the
   yield functions perform synchronization.

   [2]: The reason why it works like this is because the server read thread is
        blocked on recv, so the only way to get it to respond is to send
        something to the socket, which only the bot (client) can do.

*/

#include <foxbot/foxbot.h>
#include <foxbot/message.h>

int setup_test_server(void);
void fox_write(const char *line, ...);
void fox_shutdown(void);
void * start_listener(void *unused);
void shutdown_test_server(void);
void do_burst(int i);
void wait_for_server_notification(void);
/** Wait until the server listener has finished processing all pending
  * messages. */
void yield_to_server(void);
/** Allow the bot to run and then wait until the bot is done processing.  It
  * should generally return `BS_PAUSED`, unless the bot quits, in which case
  * `BS_QUIT` is returned. */
enum bot_status yield_to_bot(void);
/** Write a string and then wait until the bot is done processing.  The return
  * value is akin to `yield_to_bot`. */
enum bot_status write_and_wait(const char *data);
void wait_for(const char *line, ...);
void wait_for_command(enum commands cmd);
void wait_for_numeric(unsigned int numeric);
void wait_for_last_buf(const char *line, ...);
void send_broken_uint_value(void);

extern char *last_buffer;
