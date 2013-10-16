dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([sphinx], [2.2.1], [shodan(at)sphinxsearch.com])

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
	SPHINX_CFLAGS="-fno-rtti -Wall -g -D_FILE_OFFSET_BITS=64"
	SPHINX_INJECT_FLAGS="-D_FILE_OFFSET_BITS=64"
	AC_MSG_RESULT([yes])
else
	SPHINX_CFLAGS="-fno-rtti -Wall -g -D_FILE_OFFSET_BITS=64 -O3 -DNDEBUG"
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
int main() {}
#else
syntax error
#endif
#endif
], [], [AC_MSG_ERROR([Gcc version error. Minspec is 3.4])])

AC_DEFINE_UNQUOTED([COMPILER],"$CC `$CC -dumpversion`",[Define to be the name of the compiler.])
AC_DEFINE_UNQUOTED([OS_UNAME],"`uname -a`",[Full name OS])
AC_DEFINE_UNQUOTED([CONFIGURE_FLAGS],"$ac_configure_args",[Flags used to configure me])

dnl --------------------------------------------------------------------------

SPHINX_CONFIGURE_PART([checking for header files])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h sys/file.h sys/socket.h sys/time.h unistd.h pthread.h execinfo.h sys/epoll.h])
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
AC_SEARCH_LIBS([inflate],[z],[have_lz=yes],[have_lz=no])
AC_SEARCH_LIBS([logf],[m])
AC_CHECK_FUNCS([dup2 gethostbyname gettimeofday memmove memset select socket strcasecmp strchr strerror strncasecmp strnlen strstr strtol logf pread poll])
AC_CHECK_FUNCS([backtrace backtrace_symbols])

# Neither defined or undefined. So, check.
if test "z$ac_cv_func_epoll" != "zno" && test "z$ac_cv_func_epoll" != "zyes" ; then
	AC_CHECK_FUNCS(epoll_ctl, [ac_cv_func_epoll=yes], )
fi

if test "x$ac_cv_func_epoll" = "xyes" && test "x$ac_cv_header_sys_epoll_h" = "xyes" ; then
	AC_DEFINE(HAVE_EPOLL, 1,
		[Define if your system supports the epoll system calls])
	AC_LIBOBJ(epoll)
fi

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

# check if we should statically link the mysql library
AC_ARG_WITH([static-mysql], 
            AC_HELP_STRING([--with-static-mysql], [link statically with MySQL library (default is no)]),
            [ac_cv_use_static_mysql=$withval], [ac_cv_use_static_mysql=no]
)

AC_MSG_CHECKING([whether to compile with MySQL support])
if test  x$ac_cv_use_static_mysql != xno -o x$ac_cv_use_mysql != xno
then
	if test x$ac_cv_use_static_mysql != xno ; then
		AC_MSG_RESULT([static])
    	AC_CHECK_MYSQL([$ac_cv_use_static_mysql])
		MYSQL_LIBS=`echo $MYSQL_LIBS | sed -e "sX-lmysqlclientX\$MYSQL_PKGLIBDIR/libmysqlclient.aXg"`
	else
		AC_MSG_RESULT([dynamic])
		AC_CHECK_MYSQL([$ac_cv_use_mysql])
	fi
    AC_DEFINE(USE_MYSQL,1,[Define to 1 if you want to compile with MySQL support])
    AC_SUBST([MYSQL_LIBS])
    AC_SUBST([MYSQL_CFLAGS])
else
	AC_MSG_RESULT([no])
fi
AM_CONDITIONAL(USE_MYSQL, test x$ac_cv_use_mysql != xno -o x$ac_cv_use_static_mysql != xno )

dnl ---

AC_ARG_WITH([syslog],
    AC_HELP_STRING([--with-syslog], [compile with possibility to use syslog for logging (default is yes)]),
    [ac_cv_use_syslog=$withval], [ac_cv_use_syslog=yes]
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

# check if we should compile with PostgreSQL support
AC_ARG_WITH([pgsql],
            AC_HELP_STRING([--with-pgsql], [compile with PostgreSQL support (default is disabled)]),
            [ac_cv_use_pgsql=$withval], [ac_cv_use_pgsql=no]
)
# check if we should statically link the PostgreSQL library
AC_ARG_WITH([static-pgsql], 
            AC_HELP_STRING([--with-static-pgsql], [link statically with PostgreSQL library (default is no)]),
            [ac_cv_use_static_pgsql=$withval], [ac_cv_use_static_pgsql=no]
)
AC_MSG_CHECKING([whether to compile with PostgreSQL support])
if test x$ac_cv_use_static_pgsql != xno -o x$ac_cv_use_pgsql != xno
then
	if test x$ac_cv_use_static_pgsql != xno ; then
		AC_MSG_RESULT([static])
		AC_CHECK_PGSQL([$ac_cv_use_static_pgsql])
		PGSQL_LIBS=`echo $PGSQL_LIBS | sed -e "sX-lpqX$PGSQL_PKGLIBDIR/libpq.aXg"`
	else
		AC_MSG_RESULT([dynamic])
		AC_CHECK_PGSQL([ac_cv_use_pgsql])
	fi
    AC_DEFINE(USE_PGSQL,1,[Define to 1 if you want to compile with PostgreSQL support])
    AC_SUBST([PGSQL_LIBS])
    AC_SUBST([PGSQL_CFLAGS])
else
	AC_MSG_RESULT([no])
fi
AM_CONDITIONAL(USE_PGSQL, test x$ac_cv_use_pgsql != xno -o x$ac_cv_use_satic_pgsql != xno )

# add macports include directory
if (echo $MYSQL_LIBS | grep -q -- -L/opt/local/lib); then
   MYSQL_CFLAGS="$MYSQL_CFLAGS -I/opt/local/include"
fi

# get rid of possible garbage (like on mac os x)
MYSQL_CFLAGS=`echo $MYSQL_CFLAGS | sed -e 's/-O[[0123sz]]//g'`
MYSQL_CFLAGS=`echo $MYSQL_CFLAGS | sed -e 's/\-DNDEBUG//g'`

dnl ---

AC_MSG_CHECKING([whether to use 64-bit document/word IDs])
sph_enable_id64=yes
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
	ac_cv_use_internal_libstemmer=no
	AC_CHECK_LIBSTEMMER([$ac_cv_use_libstemmer])
	if test x$ac_cv_use_internal_libstemmer != xno ; then
		AC_MSG_RESULT([yes, static internal])
	else
		AC_MSG_RESULT([yes, external])
	fi
	AC_DEFINE(USE_LIBSTEMMER, 1, [libstemmer support])
	AC_SUBST([LIBSTEMMER_LIBS])
	AC_SUBST([LIBSTEMMER_CFLAGS])
else
	AC_MSG_RESULT([no])
	AC_DEFINE(USE_LIBSTEMMER, 0, [libstemmer support])
fi
AM_CONDITIONAL(USE_LIBSTEMMER, test x$ac_cv_use_libstemmer != xno)
AM_CONDITIONAL(USE_INTERNAL_LIBSTEMMER, test x$ac_cv_use_internal_libstemmer != xno)


dnl ---

# we can now set preprocessor flags for both C and C++ compilers
CPPFLAGS="$CPPFLAGS $MYSQL_CFLAGS $PGSQL_CFLAGS $LIBSTEMMER_CFLAGS"


AC_ARG_WITH([libexpat],
	AC_HELP_STRING([--with-libexpat], [compile with expat XML library suppport (default is autodetect)]),
	[ac_cv_use_expat=$withval], [ac_cv_use_expat=yes]
)


dnl ---

AC_ARG_WITH([re2],
	AC_HELP_STRING([--with-re2], [compile with re2 library support (default is disabled)]),
	[ac_cv_use_re2=$withval], [ac_cv_use_re2=no]
)

AC_MSG_CHECKING([whether to compile with re2 library support])
if test x$ac_cv_use_re2 != xno; then
	if test -d libre2 && test -f libre2/re2/re2.h; then
		AC_MSG_RESULT([yes])
		AC_DEFINE(USE_RE2, 1, [RE2 library support])
	else
		AC_MSG_ERROR([missing re2 sources from libre2])
	fi
else
	AC_MSG_RESULT([no])
	AC_DEFINE(USE_RE2, 0, [re2 library support])
fi
AM_CONDITIONAL(USE_RE2, test x$ac_cv_use_re2 != xno)

dnl ---

AC_ARG_WITH([rlp],
	AC_HELP_STRING([--with-rlp], [compile with RLP library support (default is disabled)]),
	[ac_cv_use_rlp=$withval], [ac_cv_use_rlp=no]
)

AC_MSG_CHECKING([whether to compile with RLP library support])
if test x$ac_cv_use_rlp != xno; then
	if test -d rlp && test -f rlp/rlp/include/bt_rlp_c.h; then
		AC_MSG_RESULT([yes])
		AC_DEFINE(USE_RLP, 1, [RLP library support])
	else
		AC_MSG_ERROR([missing RLP sources from librlp])
	fi
else
	AC_MSG_RESULT([no])
	AC_DEFINE(USE_RLP, 0, [RLP library support])
fi
AM_CONDITIONAL(USE_RLP, test x$ac_cv_use_rlp != xno)

dnl ---

got_expat=0
AC_MSG_CHECKING([for libexpat])
if test x$ac_cv_use_expat = xyes -a x$have_expat_h = xyes; then
	AC_SEARCH_LIBS([XML_Parse],[expat],[got_expat=1])
	if test $got_expat -eq 1; then
		AC_DEFINE([USE_LIBEXPAT],1,[define to use expat XML library])
		AC_MSG_RESULT([found])
	else
		AC_MSG_RESULT([not found])
		AC_MSG_WARN([xmlpipe2 will NOT be available])
	fi
else
	AC_MSG_RESULT([disabled])
fi

dnl ---

AC_ARG_WITH([iconv],
	AC_HELP_STRING([--with-iconv], [compile with iconv support (default is autodetect)]),
	[ac_cv_use_iconv=$withval], [ac_cv_use_iconv=yes]
)
AC_MSG_CHECKING([for libiconv])
if test x$ac_cv_use_iconv != xno -a x$have_iconv_h = xyes; then
	AC_SEARCH_LIBS([iconv],[iconv],[have_libiconv=yes],[have_libiconv=no])
	if test x$have_libiconv = xyes; then
		AC_MSG_RESULT([found])
		AC_DEFINE([USE_LIBICONV],1,[define to use iconv library])

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
		if test x$iconv_inbuf_const = xyes; then
			AC_DEFINE([ICONV_INBUF_CONST],1,[whether 2nd arg to iconv() is const ptr])
			AC_MSG_RESULT([const char **])
		else
			AC_DEFINE([ICONV_INBUF_CONST],0,[whether 2nd arg to iconv() is const ptr])
			AC_MSG_RESULT([char **])
		fi
	else
		if test $got_expat -eq 1; then
			AC_MSG_RESULT([not found])
			AC_MSG_WARN([xmlpipe2 will only support default encodings (latin-1, utf-8)])
		else
			AC_MSG_RESULT([not required and not found])
		fi
	fi
else
	AC_MSG_RESULT([disabled])
	if test $got_expat -eq 1; then
		AC_MSG_WARN([xmlpipe2 will only support default encodings (latin-1, utf-8)])
	fi
fi

dnl ---

if test x$have_zlib_h = xyes -a x$have_lz = xyes; then
   AC_DEFINE([USE_ZLIB],1,[define to use Zlib])
fi

dnl ---

AC_ARG_WITH([unixodbc],
	AC_HELP_STRING([--with-unixodbc], [compile with UnixODBC support (default is autodetect)]),
	[ac_cv_use_unixodbc=$withval], [ac_cv_use_unixodbc=yes]
)

AC_MSG_CHECKING([for UnixODBC])
if test x$ac_cv_use_unixodbc = xyes; then
	have_libodbc=no
	if test x$have_sql_h = xyes; then
		AC_SEARCH_LIBS([SQLDriverConnect],[odbc iodbc],[have_libodbc=yes])
	fi

	if test x$have_libodbc = xyes; then
		AC_MSG_RESULT([found])
		AC_DEFINE([USE_ODBC],1,[define to use ODBC library])
	else
		AC_MSG_RESULT([not found])
		AC_MSG_WARN([ODBC source support will NOT be available])
	fi
else
	AC_MSG_RESULT([disabled])
fi

dnl ---

AC_MSG_CHECKING([whether to enable c++ memory routines override])
sph_enable_mem_override=yes
AC_ARG_ENABLE([mem-override],
    [   --enable-mem-override        enable to override std c++ mem routines in production build (default is yes). Disabling may help in some rare cases as statically linking with mysql on Mac OS X Lion.],
    [sph_enable_mem_override=$enableval])
if test x$sph_enable_mem_override = xyes; then
    AC_MSG_RESULT([yes])
else
    AC_DEFINE(SPH_DONT_OVERRIDE_MEMROUTINES, 1, [Disable overriding of c++ memory routines])
    AC_MSG_RESULT([no])
fi

dnl --- 


AC_ARG_WITH([syslog],
	AC_HELP_STRING([--with-syslog], [compile with possibility to use syslog for logging (default is yes)]),
	[ac_cv_use_syslog=$withval], [ac_cv_use_syslog=yes]
)
AC_MSG_CHECKING([for Syslog])
if test x$ac_cv_use_syslog != xno; then
	if test x$have_syslog_h = xyes; then
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

if test "x$prefix" = "xNONE"; then
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
