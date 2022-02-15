dnl Process this file with autoconf to produce a configure script.
AC_INIT(pnginfo.c)
AC_CONFIG_AUX_DIR(config)
AC_REVISION
AM_INIT_AUTOMAKE(pngtools, 0.2)
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Checks for libraries.
dnl The syntax is library name, function, action if found, action if not found
dnl We just use the default if found action which adds -l<lib> to the LIBS var
dnl and #defined HAVE_LIB<lib>


dnl -lm:
AC_CHECK_LIB(m, atan)

dnl -lpng:
AC_CHECK_LIB(png, png_libpng_ver)

dnl STDC checks for stdlib.h stdarg.h string.h and float.h
AC_HEADER_STDC
AC_CHECK_HEADERS(stdio.h)

dnl Headers for libraries
AC_CHECK_HEADERS(png.h)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.
AC_FUNC_VPRINTF
dnl AC_FUNC_SNPRINTF

AC_CONFIG_FILES([Makefile man/Makefile])
AC_OUTPUT
