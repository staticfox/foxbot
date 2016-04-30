# foxbot automake file

BUILT_SOURCES = config_parser.h
CLEANFILES = config_lexer.[ch] config_parser.[ch]
AM_YFLAGS = -d

bin_PROGRAMS = foxbot
foxbot_SOURCES = channel.c \
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
                 user.h

foxbot_LDADD = libfoxbotconfigparser.a
noinst_LIBRARIES = libfoxbotconfigparser.a
libfoxbotconfigparser_a_SOURCES = config_lexer.l config_parser.y
libfoxbotconfigparser_a_CFLAGS = -Wno-sign-compare
