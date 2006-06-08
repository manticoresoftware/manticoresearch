dnl ---------------------------------------------------------------------------
dnl Macro: AC_CHECK_MYSQLR
dnl First check for custom MySQL paths in --with-mysql-* options.
dnl If some paths are missing,  check if mysql_config exists. 
dnl Then check for the libraries and replace -lmysqlclient with 
dnl -lmysqlclient_r, to enable threaded client library.
dnl ---------------------------------------------------------------------------

AC_DEFUN([AC_CHECK_MYSQLR],[
    if test [ -d "/usr/local/mysql/include" -a \
              -d "/usr/local/mysql/libmysql_r" ]
    then
        ac_cv_mysql_includes="/usr/local/mysql/include"
        ac_cv_mysql_libs="/usr/local/mysql/libmysql_r"
    elif test [ -x "/usr/local/mysql/bin/mysql_config" ]
    then
        mysqlconfig="/usr/local/mysql/bin/mysql_config"
    fi
    
# Check for custom MySQL root directory
if test [ x$1 != xyes -a x$1 != xno ] 
then
    ac_cv_mysql_root=`echo $1 | sed -e 's+/$++'`
    if test [ -d "$ac_cv_mysql_root/include" -a \
              -d "$ac_cv_mysql_root/libmysql_r" ]
    then
        ac_cv_mysql_includes="$ac_cv_mysql_root/include"
        ac_cv_mysql_libs="$ac_cv_mysql_root/libmysql_r"
    elif test [ -x "$ac_cv_mysql_root/bin/mysql_config" ]
    then
        mysqlconfig="$ac_cv_mysql_root/bin/mysql_config"
    else 
        AC_MSG_ERROR([invalid MySQL root directory: $ac_cv_mysql_root])
    fi
fi

# Check for custom includes path
if test [ -z "$ac_cv_mysql_includes" ] 
then 
    AC_ARG_WITH([mysql-includes], 
                AC_HELP_STRING([--with-mysql-includes], [path to MySQL header files]),
                [ac_cv_mysql_includes=$withval])
fi
if test [ -n "$ac_cv_mysql_includes" ]
then
    AC_CACHE_CHECK([MySQL includes], [ac_cv_mysql_includes], [ac_cv_mysql_includes=""])
    MYSQL_CFLAGS="-I$ac_cv_mysql_includes"
fi

# Check for custom library path

if test [ -z "$ac_cv_mysql_libs" ]
then
    AC_ARG_WITH([mysql-libs], 
                AC_HELP_STRING([--with-mysql-libs], [path to MySQL libraries]),
                [ac_cv_mysql_libs=$withval])
fi
if test [ -n "$ac_cv_mysql_libs" ]
then
    # Trim trailing '.libs' if user passed it in --with-mysql-libs option
    ac_cv_mysql_libs=`echo ${ac_cv_mysql_libs} | sed -e 's/.libs$//' \
                      -e 's+.libs/$++'`
    AC_CACHE_CHECK([MySQL libraries], [ac_cv_mysql_libs], [ac_cv_mysql_libs=""])
    MYSQL_LIBS="-L$ac_cv_mysql_libs -lmysqlclient_r"
fi

# If some path is missing, try to autodetermine with mysql_config
if test [ -z "$ac_cv_mysql_includes" -o -z "$ac_cv_mysql_libs" ]
then
    if test [ -z "$mysqlconfig" ]
    then 
        AC_PATH_PROG(mysqlconfig,mysql_config)
    fi
    if test [ -z "$mysqlconfig" ]
    then
        AC_MSG_ERROR([mysql_config executable not found
********************************************************************************
ERROR: cannot find MySQL libraries. If you want to compile with MySQL support,
       you must either specify file locations explicitly using 
       --with-mysql-includes and --with-mysql-libs options, or make sure path to 
       mysql_config is listed in your PATH environment variable. If you want to 
       disable MySQL support, use --without-mysql option.
********************************************************************************
])
    else
        if test [ -z "$ac_cv_mysql_includes" ]
        then
            AC_MSG_CHECKING(MySQL C flags)
            MYSQL_CFLAGS=`${mysqlconfig} --include`
            AC_MSG_RESULT($MYSQL_CFLAGS)
        fi
        if test [ -z "$ac_cv_mysql_libs" ]
        then
            AC_MSG_CHECKING(MySQL linker flags)
            MYSQL_LIBS=`${mysqlconfig} --libs | sed -e \
            's/-lmysqlclient /-lmysqlclient_r /' -e 's/-lmysqlclient$/-lmysqlclient_r/'`
            AC_MSG_RESULT($MYSQL_LIBS)
        fi
    fi
fi
])

dnl ---------------------------------------------------------------------------
dnl Macro: AC_CHECK_PGSQL
dnl First check for custom PostgreSQL paths in --with-pgsql-* options.
dnl If some paths are missing,  check if pg_config exists. 
dnl ---------------------------------------------------------------------------

AC_DEFUN([AC_CHECK_PGSQL],[

# Check for custom includes path
if test [ -z "$ac_cv_pgsql_includes" ] 
then 
    AC_ARG_WITH([pgsql-includes], 
                AC_HELP_STRING([--with-pgsql-includes], [path to PostgreSQL header files]),
                [ac_cv_pgsql_includes=$withval])
fi
if test [ -n "$ac_cv_pgsql_includes" ]
then
    AC_CACHE_CHECK([PostgreSQL includes], [ac_cv_pgsql_includes], [ac_cv_pgsql_includes=""])
    MYSQL_CFLAGS="-I$ac_cv_mysql_includes"
fi

# Check for custom library path

if test [ -z "$ac_cv_pgsql_libs" ]
then
    AC_ARG_WITH([pgsql-libs], 
                AC_HELP_STRING([--with-pgsql-libs], [path to PostgreSQL libraries]),
                [ac_cv_pgsql_libs=$withval])
fi

# If some path is missing, try to autodetermine with pgsql_config
if test [ -z "$ac_cv_pgsql_includes" -o -z "$ac_cv_pgsql_libs" ]
then
    if test [ -z "$pgconfig" ]
    then 
        AC_PATH_PROG(pgconfig,pg_config)
    fi
    if test [ -z "$pgconfig" ]
    then
        AC_MSG_ERROR([pg_config executable not found
********************************************************************************
ERROR: cannot find PostgreSQL libraries. If you want to compile with PosgregSQL support,
       you must either specify file locations explicitly using 
       --with-pgsql-includes and --with-pgsql-libs options, or make sure path to 
       pg_config is listed in your PATH environment variable. If you want to 
       disable PostgreSQL support, use --without-pgsql option.
********************************************************************************
])
    else
        if test [ -z "$ac_cv_pgsql_includes" ]
        then
            AC_MSG_CHECKING(PostgreSQL C flags)
            PGSQL_CFLAGS="-I`${pgconfig} --includedir`"
            AC_MSG_RESULT($PGSQL_CFLAGS)
        fi
        if test [ -z "$ac_cv_pgsql_libs" ]
        then
            AC_MSG_CHECKING(PostgreSQL linker flags)
            PGSQL_LIBS="-L`${pgconfig} --libdir` -lpq"
            AC_MSG_RESULT($PGSQL_LIBS)
        fi
    fi
fi
])

dnl ---------------------------------------------------------------------------
dnl Macro: AC_CHECK_AIO
dnl Check for Linux AIO availability on the target system
dnl Also, check the version of libaio library (at the moment, there are two
dnl versions with incompatible interfaces).
dnl ---------------------------------------------------------------------------

AC_DEFUN([AC_CHECK_AIO],[
if test x$enable_aio = xyes; then
    AC_CHECK_HEADER([libaio.h], 
                    [AC_DEFINE(HAVE_LIBAIO_H,1,[Define to 1 if your system has <libaio.h> header file])],
                    [enable_aio=no])
fi
if test x$enable_aio = xyes; then 
    AC_CHECK_LIB([aio], [io_queue_init], , [enable_aio=no])
fi
if test x$enable_aio = xyes; then
    AC_MSG_CHECKING(if io_getevents() has an old interface)
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
                       [[
#ifdef HAVE_LIBAIO_H
# include <libaio.h>
#endif
struct io_event event; 
io_context_t ctxt;
                       ]],
                       [[
(void)io_getevents(ctxt, 1, &event, NULL);
                       ]] )
    ], [
        AC_DEFINE([HAVE_OLD_GETEVENTS], 1, [Define to 1 if libaio has older getevents() interface])
        AC_MSG_RESULT(yes)
       ],
    [AC_MSG_RESULT(no)]
    )
fi
])

dnl ---------------------------------------------------------------------------
dnl Macro: AX_CHECK_DOCBOOK
dnl Check for availability of various DocBook utilities and perform necessary
dnl substitutions
dnl ---------------------------------------------------------------------------

AC_DEFUN([AX_CHECK_DOCBOOK], [
# It's just rude to go over the net to build
XSLTPROC_FLAGS=--nonet
DOCBOOK_ROOT=
if test ! -f /etc/xml/catalog; then
	for i in /usr/share/sgml/docbook/stylesheet/xsl/nwalsh /usr/share/sgml/docbook/xsl-stylesheets/;
	do
		if test -d "$i"; then
			DOCBOOK_ROOT=$i
		fi
	done

	# Last resort - try net
	if test -z "$DOCBOOK_ROOT"; then
		XSLTPROC_FLAGS=
	fi
else
	XML_CATALOG=/etc/xml/catalog
	CAT_ENTRY_START='<!--'
	CAT_ENTRY_END='-->'
fi

AC_CHECK_PROG(XSLTPROC,xsltproc,xsltproc,)
XSLTPROC_WORKS=no
if test -n "$XSLTPROC"; then
	AC_MSG_CHECKING([whether xsltproc works])

	if test -n "$XML_CATALOG"; then
		DB_FILE="http://docbook.sourceforge.net/release/xsl/current/xhtml/docbook.xsl"
	else
		DB_FILE="$DOCBOOK_ROOT/docbook.xsl"
	fi

	$XSLTPROC $XSLTPROC_FLAGS $DB_FILE >/dev/null 2>&1 << END
<?xml version="1.0" encoding='ISO-8859-1'?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.1.2//EN" "http://www.oasis-open.org/docbook/xml/4.1.2/docbookx.dtd">
<book id="test">
</book>
END
	if test "$?" = 0; then
		XSLTPROC_WORKS=yes
	fi
	AC_MSG_RESULT($XSLTPROC_WORKS)
fi
AM_CONDITIONAL(have_xsltproc, test "$XSLTPROC_WORKS" = "yes")

AC_SUBST(XML_CATALOG)
AC_SUBST(XSLTPROC_FLAGS)
AC_SUBST(DOCBOOK_ROOT)
AC_SUBST(CAT_ENTRY_START)
AC_SUBST(CAT_ENTRY_END)
])

dnl ---------------------------------------------------------------------------
dnl Macro: ACX_PTHREAD
dnl This macro figures out how to build C programs using POSIX threads. It sets
dnl the PTHREAD_LIBS output variable to the threads library and linker flags, 
dnl and the PTHREAD_CFLAGS output variable to any special C compiler flags that 
dnl are needed. (The user can also force certain compiler flags/libs to be 
dnl tested by setting these environment variables.) 
dnl 
dnl Also sets PTHREAD_CC to any special C compiler that is needed for 
dnl multi-threaded programs (defaults to the value of CC otherwise). 
dnl (This is necessary on AIX to use the special cc_r compiler alias.) 
dnl ---------------------------------------------------------------------------

AC_DEFUN([ACX_PTHREAD], [
AC_REQUIRE([AC_CANONICAL_HOST])
AC_LANG_SAVE
AC_LANG_C
acx_pthread_ok=no

# We used to check for pthread.h first, but this fails if pthread.h
# requires special compiler flags (e.g. on True64 or Sequent).
# It gets checked for in the link test anyway.

# First of all, check if the user has set any of the PTHREAD_LIBS,
# etcetera environment variables, and if threads linking works using
# them:
if test x"$PTHREAD_LIBS$PTHREAD_CFLAGS" != x; then
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        AC_MSG_CHECKING([for pthread_join in LIBS=$PTHREAD_LIBS with CFLAGS=$PTHREAD_CFLAGS])
        AC_TRY_LINK_FUNC(pthread_join, acx_pthread_ok=yes)
        AC_MSG_RESULT($acx_pthread_ok)
        if test x"$acx_pthread_ok" = xno; then
                PTHREAD_LIBS=""
                PTHREAD_CFLAGS=""
        fi
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
fi

# We must check for the threads library under a number of different
# names; the ordering is very important because some systems
# (e.g. DEC) have both -lpthread and -lpthreads, where one of the
# libraries is broken (non-POSIX).

# Create a list of thread flags to try.  Items starting with a "-" are
# C compiler flags, and other items are library names, except for "none"
# which indicates that we try without any flags at all, and "pthread-config"
# which is a program returning the flags for the Pth emulation library.

acx_pthread_flags="pthreads none -Kthread -kthread lthread -pthread -pthreads -mthreads pthread --thread-safe -mt pthread-config"

# The ordering *is* (sometimes) important.  Some notes on the
# individual items follow:

# pthreads: AIX (must check this before -lpthread)
# none: in case threads are in libc; should be tried before -Kthread and
#       other compiler flags to prevent continual compiler warnings
# -Kthread: Sequent (threads in libc, but -Kthread needed for pthread.h)
# -kthread: FreeBSD kernel threads (preferred to -pthread since SMP-able)
# lthread: LinuxThreads port on FreeBSD (also preferred to -pthread)
# -pthread: Linux/gcc (kernel threads), BSD/gcc (userland threads)
# -pthreads: Solaris/gcc
# -mthreads: Mingw32/gcc, Lynx/gcc
# -mt: Sun Workshop C (may only link SunOS threads [-lthread], but it
#      doesn't hurt to check since this sometimes defines pthreads too;
#      also defines -D_REENTRANT)
# pthread: Linux, etcetera
# --thread-safe: KAI C++
# pthread-config: use pthread-config program (for GNU Pth library)

case "${host_cpu}-${host_os}" in
        *solaris*)

        # On Solaris (at least, for some versions), libc contains stubbed
        # (non-functional) versions of the pthreads routines, so link-based
        # tests will erroneously succeed.  (We need to link with -pthread or
        # -lpthread.)  (The stubs are missing pthread_cleanup_push, or rather
        # a function called by this macro, so we could check for that, but
        # who knows whether they'll stub that too in a future libc.)  So,
        # we'll just look for -pthreads and -lpthread first:

        acx_pthread_flags="pthread -pthread -pthreads -mt $acx_pthread_flags"
        ;;
esac

if test x"$acx_pthread_ok" = xno; then
for flag in $acx_pthread_flags; do

        case $flag in
                none)
                AC_MSG_CHECKING([whether pthreads work without any flags])
                ;;

                -*)
                AC_MSG_CHECKING([whether pthreads work with $flag])
                PTHREAD_CFLAGS="$flag"
                ;;

                pthread-config)
                AC_CHECK_PROG(acx_pthread_config, pthread-config, yes, no)
                if test x"$acx_pthread_config" = xno; then continue; fi
                PTHREAD_CFLAGS="`pthread-config --cflags`"
                PTHREAD_LIBS="`pthread-config --ldflags` `pthread-config --libs`"
                ;;

                *)
                AC_MSG_CHECKING([for the pthreads library -l$flag])
                PTHREAD_LIBS="-l$flag"
                ;;
        esac

        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        LIBS="$PTHREAD_LIBS $LIBS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Check for various functions.  We must include pthread.h,
        # since some functions may be macros.  (On the Sequent, we
        # need a special flag -Kthread to make this header compile.)
        # We check for pthread_join because it is in -lpthread on IRIX
        # while pthread_create is in libc.  We check for pthread_attr_init
        # due to DEC craziness with -lpthreads.  We check for
        # pthread_cleanup_push because it is one of the few pthread
        # functions on Solaris that doesn't have a non-functional libc stub.
        # We try pthread_create on general principles.
        AC_TRY_LINK([#include <pthread.h>],
                    [pthread_t th; pthread_join(th, 0);
                     pthread_attr_init(0); pthread_cleanup_push(0, 0);
                     pthread_create(0,0,0,0); pthread_cleanup_pop(0); ],
                    [acx_pthread_ok=yes])

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        AC_MSG_RESULT($acx_pthread_ok)
        if test "x$acx_pthread_ok" = xyes; then
                break;
        fi

        PTHREAD_LIBS=""
        PTHREAD_CFLAGS=""
done
fi

# Various other checks:
if test "x$acx_pthread_ok" = xyes; then
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Detect AIX lossage: JOINABLE attribute is called UNDETACHED.
        AC_MSG_CHECKING([for joinable pthread attribute])
        attr_name=unknown
        for attr in PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_UNDETACHED; do
            AC_TRY_LINK([#include <pthread.h>], [int attr=$attr;],
                        [attr_name=$attr; break])
        done
        AC_MSG_RESULT($attr_name)
        if test "$attr_name" != PTHREAD_CREATE_JOINABLE; then
            AC_DEFINE_UNQUOTED(PTHREAD_CREATE_JOINABLE, $attr_name,
                               [Define to necessary symbol if this constant
                                uses a non-standard name on your system.])
        fi

        AC_MSG_CHECKING([if more special flags are required for pthreads])
        flag=no
        case "${host_cpu}-${host_os}" in
            *-aix* | *-freebsd* | *-darwin*) flag="-D_THREAD_SAFE";;
            *solaris* | *-osf* | *-hpux*) flag="-D_REENTRANT";;
        esac
        AC_MSG_RESULT(${flag})
        if test "x$flag" != xno; then
            PTHREAD_CFLAGS="$flag $PTHREAD_CFLAGS"
        fi

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        # More AIX lossage: must compile with cc_r
        AC_CHECK_PROG(PTHREAD_CC, cc_r, cc_r, ${CC})
else
        PTHREAD_CC="$CC"
fi

AC_SUBST(PTHREAD_LIBS)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_CC)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_pthread_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.]),[$1])
        :
else
        acx_pthread_ok=no
        $2
fi
AC_LANG_RESTORE
])
