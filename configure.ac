#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([xnec2c], [4.2], [nkcyham@yahoo.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/common.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
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

sinclude(acx_atlas.m4)
sinclude(acx_mkl.m4)
sinclude(acx_openblas.m4)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
pkg_modules="gtk+-3.0 >= 3.22.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)
AC_CHECK_LIB([m], [atan2])
AC_CHECK_LIB([gmodule-2.0], [g_module_open])
AC_CHECK_LIB([pthread], [pthread_create])

# Provide special options for INTEL MKL and force the use of icc
AC_MSG_CHECKING([whether INTEL's MKL is enabled])
AC_ARG_ENABLE(mkl,
	[AS_HELP_STRING([--enable-mkl],
	[Use INTEL's MKL for solvers and FFTs (default = no)])],
        enable_icc="yes"
        CC="icc"
	AC_MSG_RESULT([yes]),
	AC_MSG_RESULT([no]))
	
# Provide special options for ATLAS
AC_ARG_WITH(atlas-libdir,
	[AS_HELP_STRING([--with-atlas-libdir=<ATLAS library path>],
	[Provide an alternative path to the ATLAS library])])
AC_ARG_WITH(atlas-incdir,
	[AS_HELP_STRING([--with-atlas-incdir=<ATLAS header dir>],
	[Provide an alternative path to the ATLAS header directory])])
	
# Provide special options for OpenBLAS
AC_MSG_CHECKING([whether OpenBLAS is enabled])
AC_ARG_ENABLE(openblas,
	[AS_HELP_STRING([--enable-openblas],
	[Use the OpenBLAS library instead of ATLAS (default = no)])],
	AC_MSG_RESULT([yes]),
	AC_MSG_RESULT([no]))
AC_ARG_WITH(openblas-libdir,
	[AS_HELP_STRING([--with-openblas-libdir=<OpenBLAS library path>],
	[Provide an alternative path to the OpenBLAS library])])
AC_ARG_WITH(openblas-incdir,
	[AS_HELP_STRING([--with-openblas-incdir=<OpenBLAS header dir>],
	[Provide an alternative path to the OpenBLAS header directory])])


############ handle the INTEL MKL library (FFTW + LAPACKe) ###########
if test "$enable_mkl" = "yes"; then
  convlibs="${srcdir}/../src/wcs/libwcs_c.a,${srcdir}/../src/levmar/liblevmar.a"
  ACX_MKL($with_mkl_dir,no,$enable_best_link,$convlibs)
  AC_MSG_CHECKING([for the INTEL MKL])
  if test "$MKL_WARN" == ""; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_WARN([$MKL_WARN])
  fi
  AM_CFLAGS="$AM_CFLAGS $MKL_CFLAGS "
  AM_LDFLAGS="$AM_LDFLAGS $MKL_LDFLAGS "
  LIBS="$LIBS $MKL_LIBS"

############ Select either openblas or atlas ############
else	
  if test "x$enable_openblas" = "xyes"; then
    ######## Handle the OpenBLAS library (linear algebra: BLAS + LAPACKe) ########
    ACX_OPENBLAS($with_openblas_libdir, $with_openblas_incdir, $use_pthreads, no,
      [
        AM_CFLAGS="$AM_CFLAGS $OPENBLAS_CFLAGS "
        AM_LDFLAGS="$AM_LDFLAGS $OPENBLAS_LDFLAGS "
        LIBS="$OPENBLAS_LIBS $LIBS"
        if test "$OPENBLAS_WARN" != ""; then
          AC_MSG_WARN([$OPENBLAS_WARN])
        fi
      ],
      AC_MSG_ERROR([$OPENBLAS_ERROR Exiting.])
    )

  else
    ######### handle the ATLAS library (linear algebra: BLAS + cLAPACK) ##########
    ACX_ATLAS($with_atlas_libdir, $with_atlas_incdir, $use_pthreads,
      [
        [LIBS="$ATLAS_LIBS $LIBS"]
        if test "$ATLAS_WARN" != ""; then
	  AC_MSG_WARN([$ATLAS_WARN])
	fi
      ],
      AC_MSG_ERROR([$ATLAS_ERROR Exiting. You could try --enable-openblas if Atlas is unavailable.])
    )
  fi
fi

AC_ARG_WITH([lapack],
  [AS_HELP_STRING([--without-lapack],
  [disable support for lapack])],
  [],
  [with_lapack=no])

LIBLAPACK=
  AS_IF([test "x$with_lapack" != xno],
  [
    AX_BLAS([], [AC_MSG_ERROR([BLAS library not found])])
    AX_LAPACK([], [AC_MSG_ERROR([LAPACK library not found])])

    LIBS="$LAPACK_LIBS $BLAS_LIBS $LIBS $FLIBS"
    AC_CHECK_LIB(
    [lapack], [zgetrf], 
    [AC_SUBST([LIBLAPACK], ["$LAPACK_LIBS $BLAS_LIBS $LIBS $FLIBS"]) AC_DEFINE([LAPACK], [1], [Define if you have liblapack])],
    [AC_MSG_FAILURE([lapack library test failed (--without-lapack to disable)])],
    [])
  ])

AC_SEARCH_LIBS([dlopen], [dl dld], [], [
	AC_MSG_ERROR([unable to find the dlopen() function])
])

CFLAGS="$CFLAGS -fcx-fortran-rules"

# Internationalization tools
AM_GNU_GETTEXT_VERSION([0.19.8])
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
