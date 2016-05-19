# foxbot automake file

BUILT_SOURCES = config_parser.h
CLEANFILES = config_lexer.[ch] config_parser.[ch]
AM_YFLAGS = -d

libfoxbot_la_LIBADD = $(LIBLTDL)
libfoxbot_la_DEPENDENCIES = $(LTDLDEPS)

foxbotdir = $(pkglibdir)
foxbot_LTLIBRARIES = libfoxbot.la libfoxbotaux.la

libfoxbot_la_SOURCES = cap.c \
				 channel.c \
                 conf.c \
                 error.c \
                 foxbot.c \
                 hook.c \
                 irc_literal.c \
                 irc_numeric.c \
                 ircd.c \
                 list.c \
                 memory.c \
                 message.c \
                 module.c \
                 parser.c \
                 rope.c \
                 signal.c \
                 socket.c \
                 user.c

libfoxbotaux_la_SOURCES = config_lexer.l config_parser.y
libfoxbotaux_la_CFLAGS =
