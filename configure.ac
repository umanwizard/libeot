#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([libeot],[0.01],[brennanv@email.arizona.edu])
AM_INIT_AUTOMAKE([1.11 subdir-objects foreign dist-bzip2 dist-xz -Wall -Werror])
AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")
OLD_CFLAGS=$CFLAGS
AC_PROG_CC
AM_PROG_CC_C_O
CFLAGS=$OLD_CFLAGS
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile \
    libeot.pc \
])
AC_PROG_RANLIB
AM_PROG_AR
LT_INIT([disable-static])
AC_OUTPUT
