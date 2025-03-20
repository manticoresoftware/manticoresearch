#   Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
#
#   The MySQL Connector/C++ is licensed under the terms of the GPLv2
#   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
#   MySQL Connectors. There are special exceptions to the terms and
#   conditions of the GPLv2 as it is applied to this software, see the
#   FLOSS License Exception
#   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published
#   by the Free Software Foundation; version 2 of the License.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
#   for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
#=============================================================================
# Copyright 2015 Sphinx Technologies, Inc.
# Copyright 2017-2025, Manticore Software LTD (https://manticoresearch.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.

# The module uses these variables:
#   MYSQL_ROOT_DIR path to the MySQL bundle (where mysql is installed)
#	MYSQL_USE_STATIC_LIBS look and link with static library
#
# On exit the module will set these variables:
#
#	Mysql_FOUND - if library was found
#
# 	Mysql::Mysql - imported target to link with.
#
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)
##########################################################################

function ( _MYSQL_CONFIG VAR _opt )
	EXECUTE_PROCESS ( COMMAND ${MYSQL_CONFIG_EXECUTABLE} ${_opt} OUTPUT_VARIABLE _mysql_config_output )
	STRING ( REGEX REPLACE "[\r\n]$" "" _mysql_config_output "${_mysql_config_output}" )
	SET ( ${VAR} ${_mysql_config_output} PARENT_SCOPE )
endfunction ()

function ( _find_library VAR _names _suffixes )
	if (NOT ${VAR})
		set ( CMAKE_FIND_LIBRARY_SUFFIXES ${_suffixes} )
		find_library ( ${VAR} NAMES ${_names}
				HINTS
				${MYSQL_LIB_HINT}
				PATHS
				${MYSQL_ROOT_DIR}/lib
				${MYSQL_ROOT_DIR}/lib/mysql
				$ENV{MYSQL_ROOT_DIR}/lib
				$ENV{MYSQL_ROOT_DIR}/lib/mysql
				/usr/lib/mysql
				/usr/local/lib/mysql
				/usr/local/mysql/lib
				/usr/local/mysql/lib/mysql
				/opt/mysql/mysql/lib
				/opt/mysql/mysql/lib/mysql
				/opt/mysql-client/lib
				ENV MYSQL_DIR
				PATH_SUFFIXES
				/libmysql_r/.libs
				/lib
				/lib/mysql
				)
	endif ()
	SET ( ${VAR} ${${VAR}} PARENT_SCOPE )
endfunction ()

if (NOT MYSQL_CONFIG_EXECUTABLE)
	if (EXISTS "$ENV{MYSQL_DIR}/bin/mysql_config")
		SET ( MYSQL_CONFIG_EXECUTABLE "$ENV{MYSQL_DIR}/bin/mysql_config" )
	else ()
		set ( MYSQL_CONFIG_PREFER_PATH "$ENV{MYSQL_HOME}/bin" CACHE FILEPATH "preferred path to MySQL (mysql_config)" )
		FIND_PROGRAM ( MYSQL_CONFIG_EXECUTABLE
				NAMES mysql_config
				DOC "full path of mysql_config"
				PATHS ${MYSQL_CONFIG_PREFER_PATH}
				/usr/bin
				/usr/local/bin
				/opt/local/lib/mysql55/bin
				/opt/mysql/bin
				/opt/mysql/mysql/bin
				/usr/local/mysql/bin
				/usr/pkg/bin
				/usr/local/opt/mysql-client/bin
				${MYSQL_ROOT_DIR}/bin
				)
	endif ()
	mark_as_advanced ( MYSQL_CONFIG_EXECUTABLE )
endif ()

if (MYSQL_CONFIG_EXECUTABLE)
	_MYSQL_CONFIG ( MYSQL_INCLUDE_HINT "--variable=pkgincludedir" )
	_MYSQL_CONFIG ( MYSQL_LIB_HINT "--variable=pkglibdir" )
	mark_as_advanced ( MYSQL_INCLUDE_HINT MYSQL_LIB_HINT )
endif ()

#-------------- FIND MYSQL_INCLUDE_DIR ------------------
if (NOT MYSQL_INCLUDE_DIR)
	find_path ( MYSQL_INCLUDE_DIR mysql.h
			${MYSQL_INCLUDE_HINT}
			${WITH_MYSQL_INCLUDES}
			${MYSQL_ROOT_DIR}/include
			${MYSQL_ROOT_DIR}/include/mysql
			$ENV{MYSQL_ROOT_DIR}/include
			$ENV{MYSQL_ROOT_DIR}/include/mysql
			$ENV{MYSQL_INCLUDE_DIR}
			$ENV{MYSQL_DIR}/include
			/usr/include/mysql
			/usr/local/include/mysql
			/opt/mysql/mysql/include
			/opt/mysql/mysql/include/mysql
			/usr/local/mysql/include
			/usr/local/mysql/include/mysql
			/opt/mysql-client/include/mysql
			$ENV{ProgramFiles}/MySQL/*/include
			$ENV{SystemDrive}/MySQL/*/include
			)
endif ()

#----------------- FIND MYSQL_LIBRARY -------------------
if (MYSQL_USE_STATIC_LIBS)
	_find_library ( MYSQL_LIBRARY mysqlclient ".a;.lib" )
	mark_as_advanced ( MYSQL_INCLUDE_DIR MYSQL_LIBRARY )

	include ( FindPackageHandleStandardArgs )
	find_package_handle_standard_args ( Mysql REQUIRED_VARS MYSQL_LIBRARY MYSQL_LIBRARY )

	if (Mysql_FOUND AND NOT TARGET Mysql::Mysql)
		add_library ( Mysql::Mysql STATIC IMPORTED )
		set_target_properties ( Mysql::Mysql PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_INCLUDE_DIR}"
				IMPORTED_LOCATION "${MYSQL_LIBRARY}"
				)
	endif ()

else ()
	_find_library ( MYSQL_LIBRARY mysqlclient ".so;.dylib;.dll" )
	mark_as_advanced ( MYSQL_INCLUDE_DIR MYSQL_LIBRARY )

	include ( FindPackageHandleStandardArgs )
	find_package_handle_standard_args ( Mysql REQUIRED_VARS MYSQL_INCLUDE_DIR MYSQL_LIBRARY )

	if (Mysql_FOUND AND NOT TARGET Mysql::Mysql)
		add_library ( Mysql::Mysql SHARED IMPORTED )
		set_target_properties ( Mysql::Mysql PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_INCLUDE_DIR}"
				IMPORTED_LOCATION "${MYSQL_LIBRARY}"
				)
	endif ()
endif ()

