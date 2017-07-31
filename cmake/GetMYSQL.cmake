#=============================================================================
# Copyright 2017 Manticore Software Ltd.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# This file need to get MYSQL library.
# First it try 'traditional' way - find MYSQL package
# When found, fix CXX flags, remove any optimizations and debug mode
# from them, and provide way to load it dynamically.

# Note that this file is NOT standalone, since uses cfginfo, cfgvalue macro
# It has to be included from CMakeListst.txt where they defined.

find_package ( MYSQL )

mark_as_advanced ( MYSQL_CONFIG_EXECUTABLE MYSQL_INCLUDE_DIR MYSQL_LIB )

if ( MYSQL_FOUND )
	set ( USE_MYSQL 1 )
	include_directories ( ${MYSQL_INCLUDE_DIR} )

	# -DNDEBUG we set or reset on global level, so purge it from myqsl flags
	string ( REPLACE "-DNDEBUG" "" MYSQL_CXXFLAGS "${MYSQL_CXXFLAGS}" )
	# keep only defs, include paths and libs
	string ( REGEX MATCHALL "-[DLIl]([^ ]+)" MYSQL_CXXFLAGS "${MYSQL_CXXFLAGS}" )
	# convert list after MATCHALL back to plain string
	string ( REGEX REPLACE ";" " " MYSQL_CXXFLAGS "${MYSQL_CXXFLAGS}" )

	if ( MYSQL_CXXFLAGS )
		set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_CXXFLAGS}" )
	endif ( MYSQL_CXXFLAGS )
	CMAKE_DEPENDENT_OPTION ( DL_MYSQL "load mysql library dynamically" ON "MYSQL_FOUND;HAVE_DL;NOT STATIC_MYSQL" OFF )
	CMAKE_DEPENDENT_OPTION ( STATIC_MYSQL "link to mysql library statically" OFF "MYSQL_FOUND;NOT DL_MYSQL" OFF )
	if ( STATIC_MYSQL )
		message ( STATUS "Mysql will be linked statically" )
		string ( REGEX REPLACE "mysqlclient" "libmysqlclient.a" MYSQL_LIBRARIES "${MYSQL_LIBRARIES}" )
	endif ( STATIC_MYSQL )
	if ( DL_MYSQL )
		message ( STATUS "Mysql will not be linked (will be loaded at runtime)" )
		set ( DL_MYSQL 1 )
		cfginfo ( "DL_MYSQL=ON" )
	else ( DL_MYSQL )
		list ( APPEND EXTRA_LIBRARIES ${MYSQL_LIBRARIES} )
	endif ()
	memcfgvalues ( MYSQL_LIB MYSQL_CONFIG_EXECUTABLE )
else ( MYSQL_FOUND )
	message ( SEND_ERROR
			"********************************************************************************
ERROR: cannot find MySQL libraries. If you want to compile with MySQL support,
you must either specify file locations explicitly using
-D WITH_MYSQL_INCLUDES=... and -D WITH_MYSQL_LIBS=... options, or make sure path to
mysql_config is listed in your PATH environment variable. Or specify the path usint
-D MYSQL_CONFIG_EXECUTABLE=/path/to/mysql_config. If you want to
disable MySQL support, use -D WITH_MYSQL=OFF option.
********************************************************************************" )
endif ( MYSQL_FOUND )