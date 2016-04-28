dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(foxbot, 0.0.1, staticfox@staticfox.net)
AM_INIT_AUTOMAKE([-Wall -Werror foreign nostdinc])
AC_CONFIG_SRCDIR([foxbot.c])
AC_CONFIG_HEADERS([config.h])

dnl Checks for programs.
AC_PROG_CC_C99
AC_PROG_RANLIB
AC_PROG_YACC
AM_PROG_AR
AM_PROG_LEX

dnl Enable warnings.
AX_CFLAGS_WARN_ALL
AX_CHECK_COMPILE_FLAG([-Wextra], [AX_APPEND_FLAG([-Wextra])])
AX_CHECK_COMPILE_FLAG([-std=c99], [AX_APPEND_FLAG([-std=c99])])

dnl Checks for header files.
AC_CHECK_HEADERS([malloc.h netdb.h stdlib.h string.h sys/socket.h unistd.h])
AC_CHECK_HEADERS([fcntl.h], [], [AC_MSG_ERROR([Could not find fcntl.h.])])

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
  debug=true
  AC_MSG_RESULT(yes)
],
[
  debug=false
  AC_MSG_RESULT(no)
])

dnl memory debug spam
AC_MSG_CHECKING(whether to enable memory debugging spam)
AC_ARG_ENABLE([memdebug],
[AS_HELP_STRING([--enable-memdebug], [Print memory debugging information])],
[
  memdebug=true
  AC_DEFINE([WANT_MEMDEBUG], [1], ["Print out memory debugging information"])
  AC_MSG_RESULT(yes)
],
[
  memdebug=false
  AC_MSG_RESULT(no)
])

AM_CONDITIONAL([MEMDEBUG], [test "$memdebug" = true])

if test "$debug" = yes; then
  AX_APPEND_FLAG([-O0])
fi

AC_OUTPUT(Makefile)
