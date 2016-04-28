# foxbot automake file

AUTOMAKE_OPTIONS = foreign
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
                 foxsignal.c \
                 foxsignal.h \
                 memory.c \
                 memory.h \
                 stdinc.h
