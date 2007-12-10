dnl ---------------------------------------------------------------------------
dnl Macro: AC_CHECK_MYSQL
dnl Check for custom MySQL paths in --with-mysql-* options.
dnl If some paths are missing,  check if mysql_config exists. 
dnl ---------------------------------------------------------------------------

AC_DEFUN([AC_CHECK_MYSQL],[

user_mysql_config=
user_mysql_includes=
user_mysql_libs=

# check explicit MySQL root for mysql_config, include, lib
if test [ x$1 != xyes -a x$1 != xno ]
then
	mysqlroot=`echo $1 | sed -e 's+/$++'`
	if test [ -x "$mysqlroot/bin/mysql_config" ]
	then
		# if there's mysql_config, that's the best route
		user_mysql_config="$mysqlroot/bin/mysql_config"
	elif test [ -d "$mysqlroot/include" -a -d "$mysqlroot/lib" ]
	then
		if test [ -d "$mysqlroot/include/mysql" ]
		then
			user_mysql_includes="$mysqlroot/include/mysql"
		else
			user_mysql_includes="$mysqlroot/include"
		fi

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
for mysqlconfig in "$user_mysql_config" "mysql_config" \
	"/usr/bin/mysql_config" "/usr/local/bin/mysql_config" \
	"/usr/local/mysql/bin/mysql_config" "/opt/mysql/bin/mysql_config" "/usr/pkg/bin/mysql_config"
do
	if test [ -n "$mysqlconfig" ]
	then
		MYSQL_CFLAGS=`${mysqlconfig}z --cflags 2>/dev/null` 
		MYSQL_LIBS=`${mysqlconfig}z --libs 2>/dev/null`

		if test [ $? -eq 0 ]
		then
			AC_MSG_RESULT([$mysqlconfig])
			mysqlconfig=
			break
		else
			MYSQL_CFLAGS=
			MYSQL_LIBS=
		fi
	fi
done
if test [ -n "$mysqlconfig" ]
then
	AC_MSG_RESULT([not found])
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
	PGSQL_LIBS="-L$ac_cv_pgsql_libs -lpq"
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
