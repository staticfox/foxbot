# foxbot automake file

BUILT_SOURCES = config_parser.h
CLEANFILES = ${BUILT_SOURCES}
AM_YFLAGS = -d

bin_PROGRAMS = foxbot
foxbot_SOURCES = conf.c \
                 conf.h \
                 foxbot.c \
                 foxbot.h \
                 foxmemory.c \
                 foxmemory.h \
                 foxsignal.c \
                 foxsignal.h \
                 socket.c \
                 socket.h \
                 stdinc.h
foxbot_LDADD = libfoxbotconfigparser.a
noinst_LIBRARIES = libfoxbotconfigparser.a
libfoxbotconfigparser_a_SOURCES = config_lexer.l config_parser.y
libfoxbotconfigparser_a_CFLAGS = -Wno-sign-compare
