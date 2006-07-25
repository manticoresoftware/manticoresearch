dnl ---------------------------------------------------------------------------
dnl Macro: AC_CHECK_MYSQL
dnl Check for custom MySQL paths in --with-mysql-* options.
dnl If some paths are missing,  check if mysql_config exists. 
dnl ---------------------------------------------------------------------------

AC_DEFUN([AC_CHECK_MYSQL],[

# check well-known include paths
for MYSQL_INCLUDE in "/usr/local/mysql/include" "/usr/local/mysql/include/mysql" "/usr/include/mysql"
do
	if test [ -r "$MYSQL_INCLUDE/mysql.h" ]
	then
		ac_cv_mysql_includes=$MYSQL_INCLUDE
	fi
done

# check well-known library paths
for MYSQL_LIB in "/usr/local/mysql/lib" "/usr/local/mysql/lib/mysql" "/usr/lib/mysql"
do
	if test [ -d "$MYSQL_LIB" ]
	then
		ac_cv_mysql_libs=$MYSQL_LIB
	fi
done

# if includes and/or libs were not found,
# check well-known mysqlconfig paths
if test [ -z "$ac_cv_mysql_includes" -o -z "$ac_cv_mysql_libs" ]
then
	for MYSQL_CONFIG in "/usr/local/mysql/bin"
	do
		if test [ -x "$MYSQL_CONFIG/mysql_config" ]
		then
			mysqlconfig="$MYSQL_CONFIG/mysql_config"
		fi
	done
fi
    
# check for custom user-specified MySQL root directory
if test [ x$1 != xyes -a x$1 != xno ] 
then
	ac_cv_mysql_root=`echo $1 | sed -e 's+/$++'`
	if test [ -d "$ac_cv_mysql_root/include" -a -d "$ac_cv_mysql_root/libmysql" ]
	then
		ac_cv_mysql_includes="$ac_cv_mysql_root/include"
		ac_cv_mysql_libs="$ac_cv_mysql_root/libmysql"
	elif test [ -x "$ac_cv_mysql_root/bin/mysql_config" ]
	then
		mysqlconfig="$ac_cv_mysql_root/bin/mysql_config"
	else 
		AC_MSG_ERROR([invalid MySQL root directory: $ac_cv_mysql_root])
	fi
fi

# check for custom includes path
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

# check for custom library path
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
	MYSQL_LIBS="-L$ac_cv_mysql_libs -lmysqlclient -lz"
fi

# if some path is missing, try to autodetermine with mysql_config
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
			MYSQL_LIBS=`${mysqlconfig} --libs`
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
