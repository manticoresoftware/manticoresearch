dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

AC_INIT([sphinx], [0.9.7], [shodan@shodan.ru])

AC_CONFIG_AUX_DIR([config])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/searchd.cpp])
AC_CONFIG_HEADER([config/config.h])

# Check if we should compile with MySQL support
AC_ARG_WITH([mysql], 
            AC_HELP_STRING([--with-mysql], [compile with MySQL support (default is enabled)]),
            [ac_cv_use_mysql=$withval], [ac_cv_use_mysql=yes]
)
AC_CACHE_CHECK([whether to compile with MySQL support], [ac_cv_use_mysql], [ac_cv_use_mysql=no])


if test x$ac_cv_use_mysql != xno; then
    AC_CHECK_MYSQLR([$ac_cv_use_mysql])
    AC_DEFINE(USE_MYSQL,1,[Define to 1 if you want to compile with MySQL support])
    AC_SUBST([MYSQL_LIBS])
    AC_SUBST([MYSQL_CFLAGS])
fi
AM_CONDITIONAL(USE_MYSQL, test x$ac_cv_use_mysql != xno)


AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h sys/file.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 gethostbyname gettimeofday memmove memset select socket strcasecmp strchr strerror strncasecmp strstr strtol])


AC_CONFIG_FILES([
    Makefile
    src/Makefile
])

AC_OUTPUT

