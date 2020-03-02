AC_PREREQ([2.69])
AC_INIT([jibal], [0.0.0], [jaakko.julin@jyu.fi])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE
LT_PREREQ([2.2])
LT_INIT([dlopen])

AC_PROG_CC

AC_CHECK_HEADERS([float.h getopt.h limits.h stdint.h stdlib.h string.h math.h])

AC_TYPE_SIZE_T

AC_FUNC_MALLOC
AC_FUNC_STRTOD

AC_CHECK_FUNCS([sscanf])

AC_CHECK_LIB(m, sin)

PKG_CHECK_MODULES([GSL], [gsl >= 2.2])

AC_CONFIG_FILES([Makefile
                 jibal/Makefile
                 data/Makefile
                 src/Makefile
                 ])

CFLAGS="$CFLAGS $GSL_CFLAGS"
LIBS="$LIBS $GSL_LIBS"

AX_CREATE_PKGCONFIG_INFO
AC_OUTPUT
