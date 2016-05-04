# tests Automake file

AUTOMAKE_OPTIONS = foreign

TESTS = check_foxbot
check_PROGRAMS = check_foxbot

LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh

check_foxbot_CFLAGS = -I$(top_srcdir)/include -Wall -DTESTDIST=\"$(abs_top_srcdir)/tests/dist\" @CHECK_CFLAGS@
check_foxbot_LDADD = $(top_builddir)/src/libfoxbot.la @CHECK_LIBS@

check_foxbot_SOURCES = check_foxbot.c \
                       check_foxbot.h \
                       check_server.c \
                       check_server.h \
                       check_start.c \
                       units/channel_parse.c \
                       units/connect_parse.c \
                       units/memory_parse.c
