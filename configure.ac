dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(foxbot, 0.0.1, staticfox@staticfox.net)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([foxbot.c])
AC_CONFIG_HEADERS([config.h])

dnl Checks for programs.
AC_PROG_CC_C99
AC_PROG_YACC
AM_PROG_LEX

dnl Checks for header files.
AC_CHECK_HEADERS([malloc.h netdb.h stdlib.h string.h sys/socket.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memchr memmove memset socket strcspn strerror strstr])

dnl debugging information
AC_MSG_CHECKING(whether to enable debugging symbols)
AC_ARG_ENABLE([debug],
[  --enable-debug          Turn on debugging],
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
[  --enable-memdebug       Print memory debugging information],
[
  memdebug=true
  AC_DEFINE([WANT_MEMDEBUG], [1], ["Print out memory debugging information"])
  AC_MSG_RESULT(yes)
],
[
  memdebug=false
  AC_MSG_RESULT(no)
])

AM_CONDITIONAL([DEBUG],    [test "$debug" = "true"])
AM_CONDITIONAL([MEMDEBUG], [test "$memdebug" = "true"])

AC_OUTPUT(Makefile)
