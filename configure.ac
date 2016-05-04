dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(foxbot, 0.0.1, staticfox@staticfox.net)
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([Makefile bot/Makefile src/Makefile tests/Makefile])
AC_CONFIG_SRCDIR([src/foxbot.c])
AC_CONFIG_HEADERS([include/config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign nostdinc subdir-objects])

dnl Define directories
AC_DEFINE_DIR([PREFIX],[prefix],[Set to prefix.])
AC_DEFINE_DIR([SYSCONFDIR],[sysconfdir],[Set to sysconfdir.])
AC_DEFINE_DIR([LIBDIR],[libdir],[Set to libdir.])
AC_DEFINE_DIR([LIBEXECDIR],[libexecdir],[Set to libexecdir.])
AC_DEFINE_DIR([DATADIR],[datadir],[Set to datadir.])
AC_DEFINE_DIR([LOCALSTATEDIR],[localstatedir],[Set to localstatedir.])

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC_C99
AC_PROG_RANLIB
AC_PROG_YACC
AM_PROG_AR
AM_PROG_LEX

dnl m4 directrory
AC_CONFIG_MACRO_DIR([m4])

AC_SUBST([AM_CPPFLAGS], ['-I$(top_srcdir)/include'])
AC_SUBST([AM_CFLAGS])

dnl Enable warnings.
AX_CHECK_COMPILE_FLAG([-Werror], [WERROR=-Werror], [WERROR=])
AX_CHECK_COMPILE_FLAG([$WERROR -std=c99],
  [AX_APPEND_FLAG([-std=c99], [AM_CFLAGS])])
AX_CHECK_COMPILE_FLAG([$WERROR -Wall],
  [AX_APPEND_FLAG([-Wall], [AM_CFLAGS])])
AX_CHECK_COMPILE_FLAG([$WERROR -Wextra],
  [AX_APPEND_FLAG([-Wextra], [AM_CFLAGS])])
AX_CHECK_COMPILE_FLAG([$WERROR -pedantic],
  [AX_APPEND_FLAG([-pedantic], [AM_CFLAGS])])

dnl Checks for header files.
AC_CHECK_HEADERS([malloc.h netdb.h stdlib.h string.h sys/socket.h unistd.h])
AC_CHECK_HEADERS([fcntl.h], [], [AC_MSG_ERROR([cannot find fcntl.h.])])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([malloc_usable_size memchr memmove memset socket strcspn strerror strstr])

dnl debugging information
AC_MSG_CHECKING(whether to enable debugging symbols)
AC_ARG_ENABLE([debug],
[AS_HELP_STRING([--enable-debug], [Turn on debugging])],
[
  AX_APPEND_FLAG([-O0])
  AC_MSG_RESULT(yes)
],
[
  AC_MSG_RESULT(no)
])

dnl memory debug spam
AC_MSG_CHECKING(whether to enable memory debugging spam)
AC_ARG_ENABLE([memdebug],
[AS_HELP_STRING([--enable-memdebug], [Print memory debugging information])],
[
  AC_DEFINE([WANT_MEMDEBUG], [1], [Print out memory debugging information])
  AC_MSG_RESULT(yes)
],
[
  AC_MSG_RESULT(no)
])

dnl check for pkg-config
if test m4_ifdef([PKG_CHECK_MODULES], [yes], [no]) == no; then
  AC_MSG_ERROR([pkg-config is required.
Please install it via your system's package manager or visit pkg-config.freedesktop.org])
fi

dnl Check for libcheck
PKG_CHECK_MODULES([CHECK], [check], [HAVE_CHECK=1], [HAVE_CHECK=0])
AX_CHECK_COMPILE_FLAG([$WERROR -Wno-gnu-zero-variadic-macro-arguments],
  [AX_APPEND_FLAG([-Wno-gnu-zero-variadic-macro-arguments], [CHECK_CFLAGS])])
AC_REQUIRE_AUX_FILE([tap-driver.sh])

AC_OUTPUT
