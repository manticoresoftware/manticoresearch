dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([sphinx], [0.9.6], [shodan@shodan.ru])

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([checking build environment])

AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/searchd.cpp])
AC_CONFIG_HEADER([config/config.h])

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([configuring Sphinx])

# Check if we should compile with MySQL support
AC_ARG_WITH([mysql], 
            AC_HELP_STRING([--with-mysql], [compile with MySQL support (default is enabled)]),
            [ac_cv_use_mysql=$withval], [ac_cv_use_mysql=yes]
)
AC_CACHE_CHECK([whether to compile with MySQL support], [ac_cv_use_mysql], [ac_cv_use_mysql=no])

# Check if we should compile with PostgreSQL support
AC_ARG_WITH([pgsql],
            AC_HELP_STRING([--with-pgsql], [compile with PostgreSQL support (default is disabled)]),
            [ac_cv_use_pgsql=$withval], [ac_cv_use_pgsql=no]
)
AC_CACHE_CHECK([whether to compile with PostgreSQL support], [ac_cv_use_pgsql], [ac_cv_use_pgsql=yes])


if test x$ac_cv_use_mysql != xno; then
    AC_CHECK_MYSQLR([$ac_cv_use_mysql])
    AC_DEFINE(USE_MYSQL,1,[Define to 1 if you want to compile with MySQL support])
    AC_SUBST([MYSQL_LIBS])
    AC_SUBST([MYSQL_CFLAGS])
fi
AM_CONDITIONAL(USE_MYSQL, test x$ac_cv_use_mysql != xno)

if test x$ac_cv_use_pgsql != xno; then
    AC_CHECK_PGSQL([$ac_cv_use_pgsql])
    AC_DEFINE(USE_PGSQL,1,[Define to 1 if you want to compile with PostgreSQL support])
    AC_SUBST([PGSQL_LIBS])
    AC_SUBST([PGSQL_CFLAGS])
fi
AM_CONDITIONAL(USE_PGSQL, test x$ac_cv_use_pgsql != xno)

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([checking for compiler programs])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([checking for header files])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h sys/file.h sys/socket.h sys/time.h unistd.h])

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([checking for types])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([checking for library functions])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 gethostbyname gettimeofday memmove memset select socket strcasecmp strchr strerror strncasecmp strstr strtol])

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([generating configuration files])

CONFDIR=`eval echo "${localstatedir}"`
AC_SUBST(CONFDIR)

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    sphinx.conf.dist:sphinx.conf.in
])

AC_OUTPUT

