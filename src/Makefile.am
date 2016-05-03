# foxbot automake file

BUILT_SOURCES = config_parser.h
CLEANFILES = config_lexer.[ch] config_parser.[ch] libfoxbotconfigparser.a
AM_YFLAGS = -d

AM_CPPFLAGS = $(LTDLINCL) -I$(top_srcdir)/include -Wall
libfoxbot_la_LDFLAGS = -export-dynamic
libfoxbot_la_LIBADD = $(LIBLTDL)
libfoxbot_la_DEPENDENCIES = $(LTDLDEPS)

foxbotdir = $(pkglibdir)
foxbot_LTLIBRARIES = libfoxbot.la

libfoxbot_la_SOURCES = channel.c \
                 conf.c \
                 foxbot.c \
                 irc.c \
                 ircd.c \
                 list.c \
                 memory.c \
                 message.c \
                 signal.c \
                 socket.c \
                 user.c \
                 config_lexer.l \
                 config_parser.y
