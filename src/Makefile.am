# foxbot automake file

BUILT_SOURCES = config_parser.h
CLEANFILES = config_lexer.[ch] config_parser.[ch]
AM_YFLAGS = -d

AM_CPPFLAGS = -I$(top_srcdir)/include -Wall

noinst_LIBRARIES = libfoxbot.a libfoxbotaux.a

libfoxbot_a_SOURCES = channel.c \
                 conf.c \
                 foxbot.c \
                 irc.c \
                 ircd.c \
                 list.c \
                 memory.c \
                 message.c \
                 signal.c \
                 socket.c \
                 user.c

libfoxbotaux_a_SOURCES = config_lexer.l config_parser.y
libfoxbotaux_a_CFLAGS =
