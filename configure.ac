#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([xnec2c], [4.4.16], [xnec2c-bugs@z.ewheeler.org])
AC_CONFIG_SRCDIR([src/common.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4 m4-include])
AC_CONFIG_AUX_DIR([auto-aux])
AM_INIT_AUTOMAKE([
  1.13.2
  -Wall
])
AM_MAINTAINER_MODE([enable])

dnl ======================================================================
dnl Relevant tool release dates and versions
dnl
dnl 2012-04-24 autoconf-2.69    available in centos 7
dnl 2012-04-24 autoconf-2.69    longtime xnec2c requirement
dnl
dnl 2013-06-20 automake-1.14    introduces %reldir%, implies AM_PROG_CC_C_O
dnl 2013-06-14 automake-1.13.4  available in centos 7
dnl 2013-05-15 automake-1.13.2  (bugfix for handling multiple m4 macro dirs)
dnl 2012-12-28 automake-1.13    fix important but with AC_CONFIG_MACRO_DIRS
dnl 0000-00-00 automake-<none>  longtime xnec2c requirement
dnl
dnl 2016-06-09 gettext-0.19.8   available in centos 7
dnl 2015-12-28 gettext-0.19.7   longtime xnec2c requirement
dnl
dnl 2011-10-18 libtool-2.4.2    available in centos 7
dnl
dnl 2012-08-24 pkgconfig-0.27.1 available in centos 7
dnl
dnl 2018-04-06 glib2-2.56.1     available in centos 7
dnl
dnl ======================================================================

# AC_CANONICAL_HOST is needed to access the 'host_os' variable
AC_CANONICAL_HOST

build_linux=no
build_windows=no
build_mac=no

# Detect the target system
case "${host_os}" in
    linux*)
        build_linux=yes
        ;;
    openbsd*)
        build_openbsd=yes
        ;;
    cygwin*|mingw*)
        build_windows=yes
        ;;
    darwin*)
        build_mac=yes
        ;;
    *)
        AC_MSG_WARN([OS $host_os is not supported or has not been tested. Please open an issue here: https://github.com/KJ7LNW/xnec2c/issues])
        ;;
esac

# Pass the conditionals to automake
AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([OSX], [test "$build_mac" = "yes"])

# Set CFLAGS if not set in environment
AC_MSG_CHECKING([whether configure should try to set CFLAGS])
if test "x${CFLAGS+set}" = xset; then
   enable_cflags_setting=no
else
   enable_cflags_setting=yes
fi
AC_MSG_RESULT([$enable_cflags_setting])
test "x$enable_cflags_setting" = xyes && CFLAGS=""

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O()dnl remove this line when requiring automake >= 1.14
AC_PROG_MAKE_SET
AC_PROG_SED

AC_LANG([C])

m4_pattern_forbid([PKG_CHECK_VAR])dnl
PKG_CHECK_VAR([GLIB_COMPILE_RESOURCES], [gio-2.0], [glib_compile_resources], [], [dnl
  AC_MSG_ERROR([Could not find glib-compile-resources tool])
])
AC_MSG_CHECKING([for glib-compile-resources tool])
AC_MSG_RESULT([$GLIB_COMPILE_RESOURCES])
AS_CASE([$GLIB_COMPILE_RESOURCES], [false|no], [dnl
  AC_MSG_ERROR([glib-compile-resources tool required])
])


m4_pattern_forbid([PKG_CHECK_MODULES])dnl
PKG_CHECK_MODULES([GIO], [gio-2.0])

cat>gcrtest.xml<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<gresources>
</gresources>
EOF

AS_IF([$GLIB_COMPILE_RESOURCES --sourcedir=. --target=gcrtest.c --generate-source gcrtest.xml], [dnl
  xnc2c_gcr_works=yes
], [dnl
  xnc2c_gcr_works=no
])
AC_MSG_CHECKING([glib-compile-resources tool works])
AC_MSG_RESULT([$xnc2c_gcr_works])
AS_VAR_IF([xnc2c_gcr_works], [no], [dnl
  rm -f gcrtest.xml
  AC_MSG_ERROR([working glib-compile-resources tool required])
])

AC_MSG_CHECKING([glib-compile-resources tool output compiles and links])
xnc2c_saved_CPPFLAGS="$CPPFLAGS"
xnc2c_saved_LIBS="$LIBS"
CPPFLAGS="$CPPFLAGS $GIO_CFLAGS"
LIBS="$LDADD $GIO_LIBS"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
  #include <stdio.h>
  #include "gcrtest.c"
]], [[
  void *ptr = &static_resource;
  printf("static_resource=%p\n", ptr);
]])], [xnc2c_gcr_links=yes],
      [xnc2c_gcr_links=no])
CPPFLAGS="$xnc2c_saved_CPPFLAGS"
LIBS="$xnc2c_saved_LIBS"
AC_MSG_RESULT([$xnc2c_gcr_links])
rm -f gcrtest.c gcrtest.xml
AS_VAR_IF([xnc2c_gcr_links], [no], [dnl
  AC_MSG_FAILURE([glib-compile-resources result must compile and link])
])

# Checks for libraries.
m4_pattern_forbid([PKG_CHECK_MODULES])dnl
PKG_CHECK_MODULES([GMODULE], [gmodule-2.0])
PKG_CHECK_MODULES([GTK],     [gtk+-3.0 >= 3.18.0])

# GCC has atan2 as a built-in so the autoconf test casts it incorrectly and triggers a warning.
# This causes -Werror to fail and then xnec2c does not link with -lm.  Disable built-ins
# for this test:
CFLAGS_SAVE="$CFLAGS"
CFLAGS="$CFLAGS -fno-builtin"
AC_CHECK_LIB([m], [atan2])
CFLAGS="$CFLAGS_SAVE"

AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CHECK_LIB([pthread], [pthread_create])

# Provide special options for ATLAS
AC_ARG_WITH([atlas-incdir],
	[AS_HELP_STRING([--with-atlas-incdir=<ATLAS header dir>],
	[Provide an alternative path to the ATLAS header directory])])

# Provide special options for OpenBLAS
AC_ARG_WITH([openblas-incdir],
	[AS_HELP_STRING([--with-openblas-incdir=<OpenBLAS header dir>],
	[Provide an alternative path to the OpenBLAS header directory])])

# Check headers for linear algebra
AC_CHECK_HEADERS([clapack.h cblas.h lapacke.h openblas/cblas.h openblas/lapacke.h])

# Dynamic loader for linear algebra
AC_SEARCH_LIBS([dlopen], [dl dld], [], [dnl
  AC_MSG_ERROR([unable to find the dlopen() function])
])

# If the user sets -Werror then _FORTIFY_SOURCE=2 fails with -O0
# so make it optional:
AC_MSG_CHECKING([for _FORTIFY_SOURCE=2])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
  #define _FORTIFY_SOURCE 2
  #include <stdlib.h>
]], [[
]])], [dnl
  AC_MSG_RESULT([yes])
  CFLAGS="$CFLAGS -D_FORTIFY_SOURCE=2"
], [dnl
  AC_MSG_RESULT([no])
])

AC_CHECK_HEADER([execinfo.h], [dnl
  AC_CHECK_FUNC(backtrace, AC_DEFINE([HAVE_BACKTRACE], [1], [libc supports backtrace()]))
])


xnc2c_saved_CPPFLAGS="$CPPFLAGS"
xnc2c_saved_LIBS="$LIBS"
CPPFLAGS="$CPPFLAGS $GMODULE_CFLAGS"
LIBS="$LDADD $GMODULE_LIBS"
AC_CHECK_FUNC(g_module_supported, AC_DEFINE([HAVE_G_MODULE_SUPPORTED], [1], [we have g_module_supported()]))
CPPFLAGS="$xnc2c_saved_CPPFLAGS"
LIBS="$xnc2c_saved_LIBS"

AC_CHECK_HEADER([sys/inotify.h], [dnl
  AC_DEFINE([HAVE_INOTIFY], [1], [we have inotify])
])


# Check for OpenMP
AC_OPENMP
if test "x$OPENMP_CFLAGS" != x; then
  CFLAGS="$CFLAGS $OPENMP_CFLAGS"
  AC_DEFINE([HAVE_OPENMP], [1], [Define to 1 if you have OpenMP])
fi


## Enable fortran math optimizations

# Note that -ffinite-math-only is known to break xnec2c radiation patterns,
# so please do not add it to the list.  Interestingly, clang seems to work fine
# with -fno-honor-infinities and -fno-honor-nans which are the same as GCC's
# -ffinite-math-only flag (except in the implementation), so this seems to be
# a GCC-specific quirk.  Just in case, we are excluding the -fno-honor-infinities
# flags as well in case there is some NEC2 code that needs that handled.
# However, -fno-honor-nans should be safe because -fcx-fortran-rules disables
# NaN detection and NEC2 is originally fortran code.

m4_pattern_forbid([AX_CHECK_COMPILE_FLAG])dnl

# GCC
AX_CHECK_COMPILE_FLAG([-fcx-fortran-rules],
                      [CFLAGS="$CFLAGS -fcx-fortran-rules"])
AX_CHECK_COMPILE_FLAG([-fstack-protector-strong],
                      [CFLAGS="$CFLAGS -fstack-protector-strong"])

# clang
AX_CHECK_COMPILE_FLAG([-fno-honor-nans],
                      [CFLAGS="$CFLAGS -fno-honor-nans"])
AX_CHECK_COMPILE_FLAG([-fno-signed-zeros],
                      [CFLAGS="$CFLAGS -fno-signed-zeros"])

# gcc/clang shared
AX_CHECK_COMPILE_FLAG([-fno-math-errno],
                      [CFLAGS="$CFLAGS -fno-math-errno"])

# Internationalization tools
AM_GNU_GETTEXT_VERSION([0.19.7])
GETTEXT_PACKAGE="xnec2c"
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Gettext package.])
AM_GNU_GETTEXT([external])
PROGRAMNAME_LOCALEDIR="${datadir}/locale"
AC_SUBST([PROGRAMNAME_LOCALEDIR])

# Add the languages which your application supports here.
ALL_LINGUAS=""

# Checks for header files.
AC_CHECK_HEADERS([ctype.h gdk/gdkkeysyms.h complex.h stdio.h signal.h\
                  math.h stdlib.h unistd.h string.h fcntl.h time.h\
                  sys/types.h sys/times.h gtk/gtk.h libintl.h sys/wait.h\
                  sys/stat.h locale.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor pow select setlocale sqrt strstr])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  po/Makefile.in
])
AC_OUTPUT
