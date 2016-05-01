# foxbot automake file

BUILT_SOURCES = config_parser.h
CLEANFILES = config_lexer.[ch] config_parser.[ch] libfoxbotconfigparser.a
AM_YFLAGS = -d

libfoxbot_la_LDFLAGS = -export-dynamic -module
libfoxbot_la_LIBADD = $(LIBLTDL)
libfoxbot_la_DEPENDENCIES = $(LTDLDEPS)

foxbotdir = $(pkglibdir)
foxbot_LTLIBRARIES = libfoxbot.la

libfoxbot_la_SOURCES = channel.c \
                 channel.h \
                 conf.c \
                 conf.h \
                 foxbot.c \
                 foxbot.h \
                 irc.c \
                 irc.h \
                 ircd.c \
                 ircd.h \
                 list.c \
                 list.h \
                 memory.c \
                 memory.h \
                 message.c \
                 message.h \
                 signal.c \
                 signal.h \
                 socket.c \
                 socket.h \
                 user.c \
                 user.h \
                 config_lexer.l \
                 config_parser.y
