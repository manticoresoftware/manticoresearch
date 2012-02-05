dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([sphinx], [2.1.0], [shodan(at)sphinxsearch.com])

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

AC_COMPILE_IFELSE([
#ifdef __GNUC__
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3)
void main() {}
#else
syntax error
#endif
#endif
], [], [AC_MSG_ERROR([Gcc version error. Minspec is 3.4])])

AC_DEFINE_UNQUOTED([COMPILER],"$CC `$CC -dumpversion`",[Define to be the name of the compiler.])
AC_DEFINE_UNQUOTED([OS_UNAME],"`uname -a`",[Full name OS])

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([checking for header files])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h sys/file.h sys/socket.h sys/time.h unistd.h pthread.h execinfo.h])
AC_CHECK_HEADER(expat.h,[have_expat_h=yes],[have_expat_h=no])
AC_CHECK_HEADER(iconv.h,[have_iconv_h=yes],[have_iconv_h=no])
AC_CHECK_HEADER(zlib.h,[have_zlib_h=yes],[have_zlib_h=no])
AC_CHECK_HEADER(sql.h,[have_sql_h=yes],[have_sql_h=no])
AC_CHECK_HEADER(syslog.h,[have_syslog_h=yes],[have_syslog_h=no])

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
AC_SEARCH_LIBS([inflate],[z],[have_lz=yes],[have_lz=no])
AC_SEARCH_LIBS([logf],[m])
AC_CHECK_FUNCS([dup2 gethostbyname gettimeofday memmove memset select socket strcasecmp strchr strerror strncasecmp strstr strtol logf pread poll])
AC_CHECK_FUNCS([backtrace backtrace_symbols])

# most systems require the program be linked with librt library to use
# the function clock_gettime 
my_save_LIBS="$LIBS"
LIBS=""
AC_CHECK_LIB(rt,clock_gettime)

LIBRT=$LIBS
LIBS="$my_save_LIBS"
AC_SUBST(LIBRT)

LIBS="$LIBS $LIBRT"
AC_CHECK_FUNCS(clock_gettime)

SPHINX_CHECK_DEFINE(LOCK_EX,sys/file.h)
SPHINX_CHECK_DEFINE(F_SETLKW,fcntl.h)

# check for dlopen
# FIXME! technically, only needed in searchd
# but as UDF manager is curently in libsphinx, we link everything
AC_CHECK_LIB(dl,dlopen)
AC_CHECK_FUNCS(dlopen dlerror)

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([configuring Sphinx])

dnl ---

# check for pthreads

dnl helper that runs a test program and checks for success
pthread_prog="
#include <pthread.h>
#include <stddef.h>

void * thread_routine ( void * data )
{
	return data;
}

int main ()
{
	pthread_t thd;
	pthread_mutexattr_t mattr;
	pthread_once_t once_init = PTHREAD_ONCE_INIT;
	int data = 1;

	pthread_mutexattr_init ( &mattr );
	return pthread_create ( &thd, NULL, thread_routine, &data );
}
"

AC_DEFUN([PTHREADS_TRY_RUNCOMPILE],[
if test x$cross_compiling = xno ; then
	AC_TRY_RUN($pthread_prog, [pthreads_try_run=yes], [pthreads_try_run=no], [pthreads_try_run=no])
	if test x$pthreads_try_run = xyes ; then
		$1
	fi
else
	AC_COMPILE_IFELSE($pthread_prog, [pthreads_try_compile=yes], [pthreads_try_compile=no], [pthreads_try_compile=no])
	if test x$pthreads_try_compile = xyes ; then
		$1
	fi
fi
])


AC_DEFUN([PTHREADS_TRY_RUNLINK],[
if test x$cross_compiling = xno ; then
	AC_TRY_RUN($pthread_prog, [pthreads_try_run=yes], [pthreads_try_run=no], [pthreads_try_run=no])
	if test x$pthreads_try_run = xyes ; then
		$1
	fi
else
	AC_LINK_IFELSE($pthread_prog, [pthreads_try_link=yes], [pthreads_try_link=no], [pthreads_try_link=no])
	if test x$pthreads_try_link = xyes ; then
		$1
	fi
fi
])

# check for needed cflags
AC_CACHE_CHECK([for CFLAGS needed for pthreads], [sphinx_cv_pthreads_cflags],
[
	save_cflags=$CFLAGS
	for flag in none -kthread -pthread -pthreads -mt -mthreads -Kthread -threads; do
		CFLAGS=$save_cflags
	test "x$flag" != "xnone" && CFLAGS="$CFLAGS $flag"
		PTHREADS_TRY_RUNCOMPILE([
			sphinx_cv_pthreads_cflags="$flag"
			break
		])
	done
	CFLAGS=$save_cflags
])

if test -n "$sphinx_cv_pthreads_cflags"; then
	have_pthreads=yes
	if test "x$sphinx_cv_pthreads_cflags" != "xnone"; then
		CPPFLAGS="$CPPFLAGS $sphinx_cv_pthreads_cflags"
	fi
fi

# check for needed libs
AC_CACHE_CHECK([for LIBS needed for pthreads], [sphinx_cv_pthreads_libs], [
	save_libs=$LIBS
	for lib in -lpthread -lpthreads -lc_r; do
		LIBS="$save_libs $lib"
		PTHREADS_TRY_RUNLINK([
			sphinx_cv_pthreads_libs=$lib
			break
		])
	done
	LIBS=$save_libs
])

if test -n "$sphinx_cv_pthreads_libs"; then
	have_pthreads=yes
	LIBS="$LIBS $sphinx_cv_pthreads_libs"
fi

# final check
AC_MSG_CHECKING([for pthreads])
if test x$have_pthreads = xyes; then
	if test x$cross_compiling = xno; then
		AC_MSG_RESULT([found])
	else
		AC_MSG_RESULT([assumed as found (cross-compiling)])
	fi
else
	AC_MSG_ERROR([no working pthreads library found])
fi

AC_CHECK_FUNCS([pthread_mutex_timedlock])

dnl ---

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

# check if we should statically link the mysql library
AC_ARG_WITH([static-mysql], 
            AC_HELP_STRING([--with-static-mysql], [link statically with MySQL library (default is no)]),
            [ac_cv_use_static_mysql=$withval], [ac_cv_use_static_mysql=no]
)
AC_MSG_CHECKING([whether to link statically with MySQL support])
if test x$ac_cv_use_mysql != xno; then
if test x$ac_cv_use_static_mysql != xno; then
	AC_CHECK_MYSQL([$ac_cv_use_static_mysql])
	MYSQL_LIBS=`echo $MYSQL_LIBS | sed -e 's/\-Bdynamic/\-Bstatic/g'`
	MYSQL_LIBS="-Wl,-Bstatic $MYSQL_LIBS -Wl,-Bdynamic"
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
fi
fi

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

# add macports include directory
if (echo $MYSQL_LIBS | grep -q -- -L/opt/local/lib); then
   MYSQL_CFLAGS="$MYSQL_CFLAGS -I/opt/local/include"
fi

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

AC_ARG_WITH([iconv],
	AC_HELP_STRING([--with-iconv], [compile with iconv support (default is autodetect)]),
	[ac_cv_use_iconv=$withval], [ac_cv_use_iconv=yes]
)
AC_MSG_CHECKING([for libiconv])
if test [ $have_iconv_h = yes \
	-a $have_libiconv = yes \
	-a $got_expat -eq 1 \
	-a $ac_cv_use_iconv != no ]; \
then
	AC_DEFINE([USE_LIBICONV],1,[define to use iconv library])
	AC_MSG_RESULT([found])

	AC_MSG_CHECKING([for iconv() arg types])
	AC_LANG_PUSH([C++])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([[
			#include <iconv.h>	
			#include <stdio.h>]],
			[const char * inbuf;
			iconv_t cd;
			iconv ( cd, &inbuf, NULL, NULL, NULL );
			])],
		[iconv_inbuf_const=yes],
		[iconv_inbuf_const=no])
	AC_LANG_POP([C++])
	if test [ $iconv_inbuf_const = yes ]; then
		AC_DEFINE([ICONV_INBUF_CONST],1,[whether 2nd arg to iconv() is const ptr])
		AC_MSG_RESULT([const char **])
	else
		AC_DEFINE([ICONV_INBUF_CONST],0,[whether 2nd arg to iconv() is const ptr])
		AC_MSG_RESULT([char **])
	fi
else
	if test [ $got_expat -eq 1 ]; then
		if test [ $ac_cv_use_iconv = no ]; then
			AC_MSG_RESULT([disabled])
		else
			AC_MSG_RESULT([not found])
		fi
		AC_MSG_WARN([xmlpipe2 will only support default encodings (latin-1, utf-8)])
	else
		AC_MSG_RESULT([not required])
	fi
fi

dnl ---

if test [ $have_zlib_h = yes -a $have_lz = yes ]; then
   AC_DEFINE([USE_ZLIB],1,[define to use Zlib])
fi

dnl ---

AC_ARG_WITH([unixodbc],
	AC_HELP_STRING([--with-unixodbc], [compile with UnixODBC support (default is autodetect)]),
	[ac_cv_use_unixodbc=$withval], [ac_cv_use_unixodbc=yes]
)
AC_MSG_CHECKING([for UnixODBC])
if test [ $ac_cv_use_unixodbc != no ]; then
	if test [ $have_sql_h = yes ]; then
		AC_SEARCH_LIBS([SQLConnect],[odbc iodbc],[have_libodbc=yes],[have_libodbc=no])
		if test [ $have_libodbc = yes ]; then
			AC_DEFINE([USE_ODBC],1,[define to use ODBC library])
			AC_MSG_RESULT([found])
		else
			AC_MSG_RESULT([not found])
			AC_MSG_WARN([ODBC source support will NOT be available])
		fi
	fi
else
	AC_MSG_RESULT([disabled])
fi

dnl ---


AC_ARG_WITH([syslog],
	AC_HELP_STRING([--with-syslog], [compile with possibility to use syslog for logging (default is no)]),
	[ac_cv_use_syslog=$withval], [ac_cv_use_syslog=no]
)
AC_MSG_CHECKING([for Syslog])
if test [ $ac_cv_use_syslog != no ]; then
	if test [ $have_syslog_h = yes ]; then
		AC_DEFINE([USE_SYSLOG],1,[define to use POSIX Syslog for logging])
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
else
	AC_MSG_RESULT([disabled])
fi

dnl ---

AC_CACHE_CHECK([for unaligned RAM access],[sphinx_cv_unaligned_ram_access],[
AC_LANG_PUSH([C++])
AC_RUN_IFELSE(
	[AC_LANG_PROGRAM([[
		#include <stdlib.h>
		#include <string.h>]],
		[[char * sBuf = new char [ 8*sizeof(int) ];
		for ( int i=0; i<8*sizeof(int); i++ )
			sBuf[i] = i;

		// check for crashes (SPARC)
		volatile int iRes = 0;
		for ( int i=0; i<(int)sizeof(int); i++ )
		{
			int * pPtr = (int*)( sBuf+i );
			iRes += *pPtr;
		}

		// check for correct values (ARM)
		iRes = *(int*)( sBuf+1 );
		if (!( iRes==0x01020304 || iRes==0x04030201 ))
			return 1;
		// all seems ok
		return 0;]])],
	[sphinx_cv_unaligned_ram_access=yes],
	[sphinx_cv_unaligned_ram_access=no],
	[AC_MSG_RESULT([unknown (cross-compiling), assume no])
		sphinx_cv_unaligned_ram_access=no])
])

if test x$sphinx_cv_unaligned_ram_access = xyes ; then
	AC_DEFINE([UNALIGNED_RAM_ACCESS],1,[whether unaligned RAM access is possible])
else
	AC_DEFINE([UNALIGNED_RAM_ACCESS],0)
fi

# check endianness
AC_C_BIGENDIAN(
	AC_DEFINE(USE_LITTLE_ENDIAN, 0, [big-endian]),
	AC_DEFINE(USE_LITTLE_ENDIAN, 1, [little-endian]),
	AC_MSG_ERROR(unknown endianess not supported),
	AC_MSG_ERROR(universial endianess not supported)
)

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([generating configuration files])

if test "$prefix" = "NONE"; then
	my_op_prefix="/var"
else
	my_op_prefix="$localstatedir"
fi


CONFDIR=`eval echo "${my_op_prefix}"`
AC_SUBST(CONFDIR)

AC_CONFIG_FILES([Makefile src/Makefile libstemmer_c/Makefile doc/Makefile sphinx.conf.dist:sphinx.conf.in \
	sphinx-min.conf.dist:sphinx-min.conf.in])
AC_OUTPUT

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([configuration done])

echo "You can now run 'make install' to build and install Sphinx binaries."
echo "On a multi-core machine, try 'make -j4 install' to speed up the build."
echo
echo "Updates, articles, help forum, and commercial support, consulting, training,"
echo "and development services are available at http://sphinxsearch.com/"
echo
echo "Thank you for choosing Sphinx!"
echo
