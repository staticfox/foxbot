# tests Automake file

TESTS = check_foxbot
check_PROGRAMS = check_foxbot

LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh

check_foxbot_CPPFLAGS = $(AM_CPPFLAGS) -DTESTDIST='"$(abs_top_srcdir)/tests/dist"'
check_foxbot_CFLAGS = $(AM_CFLAGS) @CHECK_CFLAGS@
check_foxbot_LDADD = $(top_builddir)/src/libfoxbot.la $(top_builddir)/src/libfoxbotaux.la @CHECK_LIBS@

check_foxbot_SOURCES = check_foxbot.c \
                       check_foxbot.h \
                       check_server.c \
                       check_server.h \
                       check_start.c \
                       units/admin_parse.c \
                       units/cap_parse.c \
                       units/channel_parse.c \
                       units/connect_parse.c \
                       units/hook.c \
                       units/ircv3_parse.c \
                       units/memory_parse.c \
                       units/parser.c \
                       units/plugin.c \
                       units/rope.c \
                       units/user_parse.c \
                       units/wallops_parse.c
