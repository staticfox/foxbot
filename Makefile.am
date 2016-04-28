# foxbot automake file

AM_YFLAGS = -d

if DEBUG
DBG = -g
else
DBG = -O2
endif

CFLAGS = -Wall -Wextra -std=c99 $(DBG)

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
