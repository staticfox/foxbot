# foxbot automake file

BUILT_SOURCES = config_parser.h
CLEANFILES = ${BUILT_SOURCES}
AM_YFLAGS = -d

bin_PROGRAMS = foxbot
foxbot_SOURCES = conf.c \
                 conf.h \
                 config_parser.y \
                 config_lexer.l \
                 foxbot.c \
                 foxbot.h \
                 foxmemory.c \
                 foxmemory.h \
                 foxsignal.c \
                 foxsignal.h \
                 stdinc.h
