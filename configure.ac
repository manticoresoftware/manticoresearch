dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([sphinx], [0.9.8], [shodan(at)shodan.ru])

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([checking build environment])

AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/searchd.cpp])
AC_CONFIG_HEADER([config/config.h])

# hack to locate expat/iconv in /usr/local on BSD systems
CPPFLAGS="$CPPFLAGS -I/usr/local/include"
LIBS="$LIBS -L/usr/local/lib"

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([checking for compiler programs])

AC_ARG_WITH([debug],
	AC_HELP_STRING([--with-debug], [compile slower debug version (default is disabled)]),
	[ac_cv_use_debug=$withval], [ac_cv_use_debug=no]
)

AC_MSG_CHECKING([whether to compile debug version])
if test x$ac_cv_use_debug != xno; then
	SPHINX_CFLAGS="-Wall -g -D_FILE_OFFSET_BITS=64"
	SPHINX_INJECT_FLAGS="-D_FILE_OFFSET_BITS=64"
	AC_MSG_RESULT([yes])
else
	SPHINX_CFLAGS="-Wall -g -D_FILE_OFFSET_BITS=64 -O3 -DNDEBUG"
	SPHINX_INJECT_FLAGS="-D_FILE_OFFSET_BITS=64 -DNDEBUG"
	AC_MSG_RESULT([no])
fi

dnl set flags for C compiler if there are no user overrides
dnl inject required defines if there are
if test x$ac_env_CFLAGS_set != xset; then
	CFLAGS=$SPHINX_CFLAGS
else
	CFLAGS="$CFLAGS $SPHINX_INJECT_FLAGS"
fi

dnl set flags for C++ compiler if there are no user overrides
dnl inject required defines if there are
if test x$ac_env_CXXFLAGS_set != xset; then
	CXXFLAGS=$SPHINX_CFLAGS
else
	CXXFLAGS="$CXXFLAGS $SPHINX_INJECT_FLAGS"
fi

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([checking for header files])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h sys/file.h sys/socket.h sys/time.h unistd.h])
AC_CHECK_HEADER(expat.h,[have_expat_h=yes],[have_expat_h=no])
AC_CHECK_HEADER(iconv.h,[have_iconv_h=yes],[have_iconv_h=no])

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
AC_SEARCH_LIBS([setsockopt],[socket])
AC_SEARCH_LIBS([gethostbyname],[nsl socket resolv])
AC_SEARCH_LIBS([XML_Parse],[expat],[have_libexpat=yes],[have_libexpat=no])
AC_SEARCH_LIBS([iconv],[iconv],[have_libiconv=yes],[have_libiconv=no])
AC_SEARCH_LIBS([logf],[m])
AC_CHECK_FUNCS([dup2 gethostbyname gettimeofday memmove memset select socket strcasecmp strchr strerror strncasecmp strstr strtol logf])

SPHINX_CHECK_DEFINE(LOCK_EX,sys/file.h)
SPHINX_CHECK_DEFINE(F_SETLKW,fcntl.h)

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([configuring Sphinx])

# check if we should compile with MySQL support
AC_ARG_WITH([mysql], 
            AC_HELP_STRING([--with-mysql], [compile with MySQL support (default is enabled)]),
            [ac_cv_use_mysql=$withval], [ac_cv_use_mysql=yes]
)
AC_MSG_CHECKING([whether to compile with MySQL support])
if test x$ac_cv_use_mysql != xno; then
	AC_MSG_RESULT([yes])
    AC_CHECK_MYSQL([$ac_cv_use_mysql])
    AC_DEFINE(USE_MYSQL,1,[Define to 1 if you want to compile with MySQL support])
    AC_SUBST([MYSQL_LIBS])
    AC_SUBST([MYSQL_CFLAGS])
else
	AC_MSG_RESULT([no])
fi
AM_CONDITIONAL(USE_MYSQL, test x$ac_cv_use_mysql != xno)


# check if we should compile with PostgreSQL support
AC_ARG_WITH([pgsql],
            AC_HELP_STRING([--with-pgsql], [compile with PostgreSQL support (default is disabled)]),
            [ac_cv_use_pgsql=$withval], [ac_cv_use_pgsql=no]
)
AC_MSG_CHECKING([whether to compile with PostgreSQL support])
if test x$ac_cv_use_pgsql != xno; then
	AC_MSG_RESULT([yes])
    AC_CHECK_PGSQL([$ac_cv_use_pgsql])
    AC_DEFINE(USE_PGSQL,1,[Define to 1 if you want to compile with PostgreSQL support])
    AC_SUBST([PGSQL_LIBS])
    AC_SUBST([PGSQL_CFLAGS])
else
	AC_MSG_RESULT([no])
fi
AM_CONDITIONAL(USE_PGSQL, test x$ac_cv_use_pgsql != xno)


# we can now set preprocessor flags for both C and C++ compilers
CPPFLAGS="$CPPFLAGS $MYSQL_CFLAGS $PGSQL_CFLAGS"

dnl ---

AC_MSG_CHECKING([whether to use 64-bit document/word IDs])
sph_enable_id64=no
AC_ARG_ENABLE([id64],
	[  --enable-id64           use 64-bit document and word IDs (default is no)],
	[sph_enable_id64=$enableval])
if test x$sph_enable_id64 != xno; then
	AC_DEFINE(USE_64BIT, 1, [64-bit document and word IDs])
	AC_MSG_RESULT([yes])
else
	AC_DEFINE(USE_64BIT, 0, [64-bit document and word IDs])
	AC_MSG_RESULT([no])
fi

dnl ---

AC_ARG_WITH([libstemmer],
	AC_HELP_STRING([--with-libstemmer], [compile with libstemmer support (default is disabled)]),
	[ac_cv_use_libstemmer=$withval], [ac_cv_use_libstemmer=no]
)

AC_MSG_CHECKING([whether to compile with libstemmer support])
if test x$ac_cv_use_libstemmer != xno; then
	if test -d libstemmer_c && test -f libstemmer_c/include/libstemmer.h; then
		AC_MSG_RESULT([yes])
		AC_DEFINE(USE_LIBSTEMMER, 1, [libstemmer support])
	else
		AC_MSG_ERROR([missing libstemmer sources from libstemmer_c.

Please download the C version of libstemmer library from
http://snowball.tartarus.org/ and extract its sources over libstemmer_c/
subdirectory in order to build Sphinx with libstemmer support.
])
	fi
else
	AC_MSG_RESULT([no])
	AC_DEFINE(USE_LIBSTEMMER, 0, [libstemmer support])
fi
AM_CONDITIONAL(USE_LIBSTEMMER, test x$ac_cv_use_libstemmer != xno)

dnl ---

got_expat=0
AC_MSG_CHECKING([for libexpat])
if test [ $have_expat_h = yes -a $have_libexpat = yes ]; then
	AC_DEFINE([USE_LIBEXPAT],1,[define to use expat XML library])
	AC_MSG_RESULT([found])
	got_expat=1
else
	AC_MSG_RESULT([not found])
	AC_MSG_WARN([xmlpipe2 will NOT be available])
fi

dnl ---

AC_MSG_CHECKING([for libiconv])
if test [ $have_iconv_h = yes -a $have_libiconv = yes ]; then
	AC_DEFINE([USE_LIBICONV],1,[define to use iconv library])
	AC_MSG_RESULT([found])
else
	AC_MSG_RESULT([not found])
	if test [ $got_expat -eq 1 ]; then
		AC_MSG_WARN([xmlpipe2 will only support default encodings (latin-1, utf-8)])
	fi
fi

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([generating configuration files])

if test "$prefix" = "NONE"; then
	my_op_prefix="/var"
else
	my_op_prefix="$localstatedir"
fi


CONFDIR=`eval echo "${my_op_prefix}"`
AC_SUBST(CONFDIR)

AC_CONFIG_FILES([Makefile src/Makefile libstemmer_c/Makefile sphinx.conf.dist:sphinx.conf.in])
AC_OUTPUT

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([configuration done])

echo You can now run \'make\' to build Sphinx binaries,
echo and then run \'make install\' to install them.
echo
