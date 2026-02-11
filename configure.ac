dnl Process this file with autoconf to produce a configure script.
AC_INIT(pngtools, [1.0])
AC_CONFIG_SRCDIR([pnginfo.c])
AC_CONFIG_AUX_DIR(config)
AC_REVISION
AM_INIT_AUTOMAKE
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
AC_SEARCH_LIBS(png_read_image, png, [], [AC_MSG_ERROR([libpng not found])])

dnl Headers for libraries
AC_CHECK_HEADERS(png.h)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.
AC_FUNC_VPRINTF
dnl AC_FUNC_SNPRINTF

AC_CONFIG_FILES([Makefile man/Makefile
                 man/pngchunkdesc.sgml
                 man/pngchunks.sgml
                 man/pngcp.sgml
                 man/pnginfo.sgml])
AC_OUTPUT
