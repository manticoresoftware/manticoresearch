dnl ---------------------------------------------------------------------------
dnl Macro: AC_CHECK_MYSQL
dnl Check for custom MySQL paths in --with-mysql-* options.
dnl If some paths are missing,  check if mysql_config exists. 
dnl ---------------------------------------------------------------------------

AC_DEFUN([AC_CHECK_MYSQL],[

mysqlconfig_locations="mysql_config /usr/bin/mysql_config /usr/local/bin/mysql_config /usr/local/mysql/bin/mysql_config /opt/mysql/bin/mysql_config /usr/pkg/bin/mysql_config"
user_mysql_includes=
user_mysql_libs=

# check explicit MySQL root for mysql_config, include, lib
if test [ x$1 != xyes -a x$1 != xno ]
then
	mysqlroot=`echo $1 | sed -e 's+/$++'`
	if test [ -x "$mysqlroot/bin/mysql_config" ]
	then
		# if there's mysql_config, that's the best route
		mysqlconfig_locations="$mysqlroot/bin/mysql_config"
	elif test [ -d "$mysqlroot/include" -a -d "$mysqlroot/lib" ]
	then
		# explicit root; do not check well-known paths
		mysqlconfig_locations=

		# includes
		if test [ -d "$mysqlroot/include/mysql" ]
		then
			user_mysql_includes="$mysqlroot/include/mysql"
		else
			user_mysql_includes="$mysqlroot/include"
		fi

		# libs
		if test [ -d "$mysqlroot/lib/mysql" ]
		then
			user_mysql_libs="$mysqlroot/lib/mysql"
		else
			user_mysql_libs="$mysqlroot/lib"
		fi
	else 
		AC_MSG_ERROR([invalid MySQL root directory '$mysqlroot'; neither bin/mysql_config, nor include/ and lib/ were found there])
	fi
fi


# try running mysql_config
AC_MSG_CHECKING([for mysql_config])
for mysqlconfig in $mysqlconfig_locations
do
	if test [ -n "$mysqlconfig" ]
	then
		MYSQL_CFLAGS=`${mysqlconfig} --cflags 2>/dev/null` 
		MYSQL_LIBS=`${mysqlconfig} --libs 2>/dev/null`

		if test [ $? -eq 0 ]
		then
			MYSQL_PKGLIBDIR=`echo $MYSQL_LIBS | sed -e 's/-[[^L]][[^ ]]*//g;s/\s*-L//g;'`
			AC_MSG_RESULT([$mysqlconfig])
			mysqlconfig=
			break
		else
			MYSQL_CFLAGS=
			MYSQL_LIBS=
			MYSQL_PKGLIBDIR=
		fi
	fi
done
if test [ -n "$mysqlconfig" ]
then
	mysqlconfig_used=
	AC_MSG_RESULT([not found])
else
	mysqlconfig_used=yes
fi


# if there's nothing from mysql_config, check well-known include paths
# explicit overrides will be applied later
if test [ -z "$MYSQL_CFLAGS" ]
then
	for CANDIDATE in "$user_mysql_includes" "/usr/local/mysql/include" "/usr/local/mysql/include/mysql" \
		"/usr/include/mysql"
	do
		if test [ -n "$CANDIDATE" -a -r "$CANDIDATE/mysql.h" ]
		then
			MYSQL_CFLAGS="-I$CANDIDATE"
			break
		fi
	done
fi


# if there's nothing from mysql_config, check well-known library paths
# explicit overrides will be applied later
if test [ -z "$MYSQL_LIBS" ]
then
	for CANDIDATE in "$user_mysql_libs" "/usr/lib64/mysql" \
		"/usr/local/mysql/lib/mysql" "/usr/local/mysql/lib" \
		"/usr/local/lib/mysql" "/usr/lib/mysql" \
		"/opt/mysql/lib/mysql" "/usr/pkg/lib/mysql"
	do
		if test [ -n "$CANDIDATE" -a -d "$CANDIDATE" ]
		then
			MYSQL_LIBS="-L$CANDIDATE -lmysqlclient -lz"
			MYSQL_PKGLIBDIR="$CANDIDATE"
			break
		fi
	done
fi


# apply explicit include path overrides
AC_ARG_WITH([mysql-includes],
	AC_HELP_STRING([--with-mysql-includes], [path to MySQL header files]),
	[ac_cv_mysql_includes=$withval])
if test [ -n "$ac_cv_mysql_includes" ]
then
	MYSQL_CFLAGS="-I$ac_cv_mysql_includes"
fi


# apply explicit lib path overrides
AC_ARG_WITH([mysql-libs], 
	AC_HELP_STRING([--with-mysql-libs], [path to MySQL libraries]),
	[ac_cv_mysql_libs=$withval])
if test [ -n "$ac_cv_mysql_libs" ]
then
	# Trim trailing '.libs' if user passed it in --with-mysql-libs option
	ac_cv_mysql_libs=`echo ${ac_cv_mysql_libs} | sed -e 's/.libs$//' \
		-e 's+.libs/$++'`
	MYSQL_LIBS="-L$ac_cv_mysql_libs -lmysqlclient -lz"
	MYSQL_PKGLIBDIR="$ac_cv_mysql_libs"
fi

# if we got options from mysqlconfig try to actually use them
if test [ -n "$mysqlconfig_used" -a -n "$MYSQL_CFLAGS" -a -n "$MYSQL_LIBS" ]
then
	_CFLAGS=$CFLAGS
	_LIBS=$LIBS
	
	CFLAGS="$CFLAGS $MYSQL_CFLAGS"
	LIBS="$LIBS $MYSQL_LIBS"
	
	AC_CHECK_FUNC(mysql_real_connect,[],
	[
		# if mysql binary was built using a different compiler and we
		# got options from mysql_config some of them might not work
		# with compiler we will be using
		
		# throw away everything that isn't one of -D -L -I -l and retry
		MYSQL_CFLAGS=`echo $MYSQL_CFLAGS | sed -e 's/-[[^DLIl]][[^ ]]*//g'`
		MYSQL_LIBS=`echo $MYSQL_LIBS | sed -e 's/-[[^DLIl]][[^ ]]*//g'`

		CFLAGS="$_CFLAGS $MYSQL_CFLAGS"
		LIBS="$_LIBS $MYSQL_LIBS"

		unset ac_cv_func_mysql_real_connect
		AC_CHECK_FUNC(mysql_real_connect,[],
		[
			# ... that didn't help
			# clear flags, the code below will complain
			MYSQL_CFLAGS=
			MYSQL_LIBS=
			MYSQL_PKGLIBDIR=
		])
	])
	CFLAGS=$_CFLAGS
	LIBS=$_LIBS
fi

# now that we did all we could, perform final checks
AC_MSG_CHECKING([MySQL include files])
if test [ -z "$MYSQL_CFLAGS" ]
then
	AC_MSG_ERROR([missing include files.

******************************************************************************
ERROR: cannot find MySQL include files.

Check that you do have MySQL include files installed.
The package name is typically 'mysql-devel'.

If include files are installed on your system, but you are still getting
this message, you should do one of the following:

1) either specify includes location explicitly, using --with-mysql-includes;
2) or specify MySQL installation root location explicitly, using --with-mysql;
3) or make sure that the path to 'mysql_config' program is listed in
   your PATH environment variable.

To disable MySQL support, use --without-mysql option.
******************************************************************************
])
else
	AC_MSG_RESULT([$MYSQL_CFLAGS])
fi


AC_MSG_CHECKING([MySQL libraries])
if test [ -z "$MYSQL_LIBS" ]
then
	AC_MSG_ERROR([missing libraries.

******************************************************************************
ERROR: cannot find MySQL libraries.

Check that you do have MySQL libraries installed.
The package name is typically 'mysql-devel'.

If libraries are installed on your system, but you are still getting
this message, you should do one of the following:

1) either specify libraries location explicitly, using --with-mysql-libs;
2) or specify MySQL installation root location explicitly, using --with-mysql;
3) or make sure that the path to 'mysql_config' program is listed in
   your PATH environment variable.

To disable MySQL support, use --without-mysql option.
******************************************************************************
])
else
	AC_MSG_RESULT([$MYSQL_LIBS])
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
	PGSQL_CFLAGS="-I$ac_cv_pgsql_includes"
fi

# Check for custom library path
if test [ -z "$ac_cv_pgsql_libs" ]
then
	AC_ARG_WITH([pgsql-libs], 
		AC_HELP_STRING([--with-pgsql-libs], [path to PostgreSQL libraries]),
		[ac_cv_pgsql_libs=$withval])
fi
if test [ -n "$ac_cv_pgsql_libs" ]
then
	AC_CACHE_CHECK([PostgreSQL libraries], [ac_cv_pgsql_libs], [ac_cv_pgsql_libs=""])
	PGSQL_PKGLIBDIR="$ac_cv_pgsql_libs"
	PGSQL_LIBS="-L$PGSQL_PKGLIBDIR -lpq"
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
			PGSQL_PKGLIBDIR=`${pgconfig} --libdir`
            PGSQL_LIBS="-L$PGSQL_PKGLIBDIR -lpq"
            AC_MSG_RESULT($PGSQL_LIBS)
        fi
    fi
fi
])

dnl ---------------------------------------------------------------------------
dnl Macro: AC_CHECK_LIBSTEMMER
dnl Check the libstemmer first in custom include path in --with-libstemmer=*
dnl If not given, try to guess common shared libs, and finally fall back into
dnl old sphinx way which invokes statically linked lib built from the sources
dnl ---------------------------------------------------------------------------

AC_DEFUN([AC_CHECK_LIBSTEMMER],[

# cflags and libs
LIBSTEMMER_CFLAGS=
LIBSTEMMER_LIBS=

# First check, if we have the sources of internal libstemmer.
# If so, it has the max. priority over any other possibilities
if test -d ./libstemmer_c && test -f libstemmer_c/include/libstemmer.h; then
	ac_cv_use_internal_libstemmer=yes
    LIBSTEMMER_LIBS="\$(top_srcdir)/libstemmer_c/libstemmer.a"
    LIBSTEMMER_CFLAGS="-I\$(top_srcdir)/libstemmer_c/include"
else

# possible includedir paths
includedirs="/usr/include /usr/include/libstemmer /usr/include/libstemmer_c"

# possible libdirs -- 64bit first in case of multiarch environments
libdirs="/usr/lib/x86_64-linux-gnu /usr/lib64 /usr/local/lib64 /usr/lib/i386-linux-gnu /usr/lib /usr/local/lib"

# possible libnames -- shared one first, then static one
libnames="stemmer stemmer_c"

# was (include) path explicitely given?
if test [ -n "$ac_cv_use_libstemmer" -a x$ac_cv_use_libstemmer != xyes]; then
       includedirs=$ac_cv_use_libstemmer
fi


# try to find header files
for includedir in $includedirs
do
       if test [ -f $includedir/libstemmer.h ]; then
               LIBSTEMMER_CFLAGS="-I$includedir"
               break
       fi
done

# try to find shared library
for libname in $libnames
do
       for libdir in $libdirs
       do
               if test [ -f $libdir/lib${libname}.so ]; then
                       LIBSTEMMER_LIBS="-L$libdir -l$libname"
                       break 2
               fi
       done
done

# if not found, check static libs
if test [ -z "$LIBSTEMMER_LIBS" ]; then
       for libname in $libnames
       do
               for libdir in $libdirs
               do
                       if test [ -f $libdir/lib${libname}.a ]; then
                               LIBSTEMMER_LIBS="$libdir/lib${libname}.a"
                               break 2
                       fi
               done
       done
fi
fi

# Now we either have libstemmer, or not
if test [ -z "$LIBSTEMMER_LIBS" ]; then
	AC_MSG_ERROR([missing libstemmer sources from libstemmer_c.

Please download the C version of libstemmer library from
http://snowball.tartarus.org/ and extract its sources over libstemmer_c/
subdirectory in order to build Sphinx with libstemmer support. Or
install the package named like 'libstemmer-dev' using your favorite
package manager.
])
fi

])

dnl ---------------------------------------------------------------------------
dnl Macro: AC_CHECK_RE2
dnl Check user-specified path in --with-re2=*, then check ./libre2 path.
dnl Finally check for installed libraries and headers if present.
dnl Also libraries and headers paths can be given using --with-re2-libs and
dnl --with-re2-includes
dnl ---------------------------------------------------------------------------

AC_DEFUN([AC_CHECK_RE2],[

# Includes and libraries
LIBRE2_CFLAGS=
LIBRE2_LIBS=

# First check if include path was explicitly given.
# If so, it has the maximum priority over any other possibilities

if test [ -n "$ac_cv_use_re2" -a x$ac_cv_use_re2 != xyes]; then
       re2include=$ac_cv_use_re2
       if test [ -f $re2include/re2/re2.h ]; then
               LIBRE2_CFLAGS="-I$re2include"
               LIBRE2_LIBS="$re2include/obj/libre2.a"

# Use re2 Makefile if present
               if test [ -f $re2include/Makefile ]; then
                       LIBRE2_PATH="$re2include"
               fi
       fi
else

# Check if there any sources in ./libre2 path
       if test -d ./libre2 && test -f ./libre2/re2/re2.h; then
               ac_cv_use_re2=yes
               LIBRE2_LIBS="\$(top_srcdir)/libre2/obj/libre2.a"
               LIBRE2_CFLAGS="-I\$(top_srcdir)/libre2"
               LIBRE2_PATH="libre2"
       else

# Possible include paths
       re2includedirs="/usr/include /usr/include/re2"

# Possible libraries paths
       re2libdirs="/usr/lib/x86_64-linux-gnu /usr/lib64 /usr/local/lib64 /usr/lib/i386-linux-gnu /usr/lib /usr/local/lib"


# Trying to find installed header files
       for re2includedir in $re2includedirs
       do
               if test [ -f $re2includedir/re2/re2.h ]; then
                       LIBRE2_CFLAGS="-I$re2includedir"
                       break
               fi
       done

# Trying to find installed libraries
       for re2libdir in $re2libdirs
       do
               if test [ -f $re2libdir/libre2.a ]; then
                       LIBRE2_LIBS="$re2libdir/libre2.a"
                       break 2
               fi
       done

fi
fi

# Apply explicit include path overrides

if test x$ac_cv_re2_includes != xno; then
       if test [ -f $ac_cv_re2_includes/re2/re2.h ]; then
               LIBRE2_CFLAGS="-I$ac_cv_re2_includes"
       else
               AC_MSG_ERROR([missing re2 headers])
       fi
fi


# Apply explicit lib path overrides

if test x$ac_cv_re2_libs != xno; then
       if test  [ -f "$ac_cv_re2_libs" ]; then
               LIBRE2_LIBS="$ac_cv_re2_libs"
       else
               AC_MSG_ERROR([missing re2 library libre2.a])
       fi
fi


# Now we either have re2, or not
if test [ -z "$LIBRE2_LIBS" ]; then
        AC_MSG_ERROR([missing re2 sources])
fi

])

dnl ---------------------------------------------------------------------------
dnl Macro: SPHINX_CONFIGURE_PART
dnl
dnl Tells what stage is ./configure running now, nicely formatted
dnl ---------------------------------------------------------------------------

dnl SPHINX_CONFIGURE_PART(MESSAGE)
AC_DEFUN([SPHINX_CONFIGURE_PART],[
	AC_MSG_RESULT()
	AC_MSG_RESULT([$1])
	TMP=`echo $1 | sed -e sX.X-Xg`
	AC_MSG_RESULT([$TMP])
	AC_MSG_RESULT()
])

dnl ---------------------------------------------------------------------------
dnl Macro: SPHINX_CHECK_DEFINE
dnl
dnl Checks if this symbol is defined in that header file
dnl ---------------------------------------------------------------------------

AC_DEFUN([SPHINX_CHECK_DEFINE],[
	AC_CACHE_CHECK([for $1 in $2],ac_cv_define_$1,[
		AC_EGREP_CPP(YES_IS_DEFINED, [
#include <$2>
#ifdef $1
YES_IS_DEFINED
#endif
		], ac_cv_define_$1=yes, ac_cv_define_$1=no)
	])
	if test "$ac_cv_define_$1" = "yes"; then
		AC_DEFINE(HAVE_$1, 1, [Define if $1 is defined in $2])
	fi
])
