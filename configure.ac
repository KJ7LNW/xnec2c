#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([xnec2c], [4.3.4], [xnec2c-bugs@z.ewheeler.org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/common.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4 m4-include])
AM_MAINTAINER_MODE

# Set CFLAGS if not set in environment
AC_MSG_CHECKING([whether configure should try to set CFLAGS])
if test "x${CFLAGS+set}" = xset; then
   enable_cflags_setting=no
else
   enable_cflags_setting=yes
fi
AC_MSG_RESULT($enable_cflags_setting)
test "x$enable_cflags_setting" = xyes && CFLAGS=""

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
pkg_modules="gtk+-3.0 >= 3.18.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)
AC_CHECK_LIB([m], [atan2])
AC_CHECK_LIB([gmodule-2.0], [g_module_open])
AC_CHECK_LIB([pthread], [pthread_create])

# Provide special options for ATLAS
AC_ARG_WITH(atlas-incdir,
	[AS_HELP_STRING([--with-atlas-incdir=<ATLAS header dir>],
	[Provide an alternative path to the ATLAS header directory])])
	
# Provide special options for OpenBLAS
AC_ARG_WITH(openblas-incdir,
	[AS_HELP_STRING([--with-openblas-incdir=<OpenBLAS header dir>],
	[Provide an alternative path to the OpenBLAS header directory])])

# Check headers for linear algebra
AC_CHECK_HEADERS([clapack.h cblas.h lapacke.h openblas/cblas.h openblas/lapacke.h])

# Dynamic loader for linear algebra  
AC_SEARCH_LIBS([dlopen], [dl dld], [], [
	AC_MSG_ERROR([unable to find the dlopen() function])
])

# Check for OpenMP
AC_OPENMP
if test "x$OPENMP_CFLAGS" != "x"; then
	CFLAGS="$CFLAGS $OPENMP_CFLAGS"
	AC_DEFINE(HAVE_OPENMP, 1 , Define to 1 if you have OpenMP )
fi


## Enable fortran math optimizations

# GCC
AX_CHECK_COMPILE_FLAG(-fcx-fortran-rules, CFLAGS="$CFLAGS -fcx-fortran-rules")

# clang
AX_CHECK_COMPILE_FLAG(-fno-honor-infinities, CFLAGS="$CFLAGS -fno-honor-infinities")
AX_CHECK_COMPILE_FLAG(-fno-honor-nans, CFLAGS="$CFLAGS -fno-honor-nans")
AX_CHECK_COMPILE_FLAG(-ffinite-math-only, CFLAGS="$CFLAGS -ffinite-math-only")
AX_CHECK_COMPILE_FLAG(-fno-signed-zeros, CFLAGS="$CFLAGS -fno-signed-zeros")
AX_CHECK_COMPILE_FLAG(-fno-math-errno, CFLAGS="$CFLAGS -fno-math-errno")

# Internationalization tools
AM_GNU_GETTEXT_VERSION([0.19.7])
GETTEXT_PACKAGE=xnec2c
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Gettext package.])
AM_GNU_GETTEXT([external])
PROGRAMNAME_LOCALEDIR=[${datadir}/locale]
AC_SUBST(PROGRAMNAME_LOCALEDIR)

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

AC_CONFIG_FILES([Makefile
                 src/Makefile
				 po/Makefile.in])
AC_OUTPUT
