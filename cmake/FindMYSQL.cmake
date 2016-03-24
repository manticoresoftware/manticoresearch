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
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.

# The module uses these variables:
#   WITH_MYSQL_INCLUDES path to MySQL header files
#	WITH_MYSQL_LIBS path to MySQL library
#	WITH_MYSQL_ROOT path to the MySQL bundle (where both header and library lives)

# On exit the module will set these variables:

#	MySQL Include dir: MYSQL_INCLUDE_DIR
#	MySQL Library dir: MYSQL_LIB_DIR
#	MySQL CXXFLAGS: MYSQL_CXXFLAGS
#	MySQL Link flags: MYSQL_LINK_FLAGS
#	MySQL dynamic load test library: MYSQLCPPCONN_DYNLOAD_MYSQL_LIB

#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)
##########################################################################
MACRO(_MYSQL_CONFIG VAR _regex _opt)
	EXECUTE_PROCESS(COMMAND ${MYSQL_CONFIG_EXECUTABLE} ${_opt}
			OUTPUT_VARIABLE _mysql_config_output
			)
	SET(_var ${_mysql_config_output})
	STRING(REGEX MATCHALL "${_regex}([^ ]+)" _mysql_config_output "${_mysql_config_output}")
	STRING(REGEX REPLACE "^[ \t]+" "" _mysql_config_output "${_mysql_config_output}")
	STRING(REGEX REPLACE "[\r\n]$" "" _mysql_config_output "${_mysql_config_output}")
	STRING(REGEX REPLACE "${_regex}" "" _mysql_config_output "${_mysql_config_output}")
	SEPARATE_ARGUMENTS(_mysql_config_output)
    SET(${VAR} ${_mysql_config_output})
ENDMACRO(_MYSQL_CONFIG _regex _opt)


IF (NOT MYSQL_CONFIG_EXECUTABLE)
	IF (EXISTS "$ENV{MYSQL_DIR}/bin/mysql_config")
		SET(MYSQL_CONFIG_EXECUTABLE "$ENV{MYSQL_DIR}/bin/mysql_config")
	ELSE (EXISTS "$ENV{MYSQL_DIR}/bin/mysql_config")
		FIND_PROGRAM(MYSQL_CONFIG_EXECUTABLE
			NAMES mysql_config
			DOC "full path of mysql_config"
			PATHS	/usr/bin
				/usr/local/bin
				/opt/local/lib/mysql55/bin
				/opt/mysql/bin
				/opt/mysql/mysql/bin
				/usr/local/mysql/bin
				/usr/pkg/bin
				${WITH_MYSQL_ROOT}/bin
		)
	ENDIF (EXISTS "$ENV{MYSQL_DIR}/bin/mysql_config")
ENDIF (NOT MYSQL_CONFIG_EXECUTABLE)

#-------------- FIND MYSQL_INCLUDE_DIR ------------------
SET(MYSQL_CXXFLAGS "")
IF(MYSQL_CONFIG_EXECUTABLE)
	_MYSQL_CONFIG(MYSQL_INCLUDE_DIR "(^| )-I" "--include")
	MESSAGE(STATUS "mysql_config was found ${MYSQL_CONFIG_EXECUTABLE}")
	EXECUTE_PROCESS(COMMAND ${MYSQL_CONFIG_EXECUTABLE} "--cflags"
					OUTPUT_VARIABLE _mysql_config_output
					)
	STRING(REGEX MATCHALL "-m([^\r\n]+)" MYSQL_LINK_FLAGS "${_mysql_config_output}")
	STRING(REGEX REPLACE "[\r\n]$" "" MYSQL_CXXFLAGS "${_mysql_config_output}")
#	ADD_DEFINITIONS("${MYSQL_CXXFLAGS}")
ELSE (MYSQL_CONFIG_EXECUTABLE)
	MESSAGE(STATUS "ENV{MYSQL_DIR} = $ENV{MYSQL_DIR}")
	FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
	        ${WITH_MYSQL_INCLUDES}
	        ${WITH_MYSQL_ROOT}/include
	        ${WITH_MYSQL_ROOT}/include/mysql
			$ENV{MYSQL_INCLUDE_DIR}
			$ENV{MYSQL_DIR}/include
			/usr/include/mysql
			/usr/local/include/mysql
			/opt/mysql/mysql/include
			/opt/mysql/mysql/include/mysql
			/usr/local/mysql/include
			/usr/local/mysql/include/mysql
			$ENV{ProgramFiles}/MySQL/*/include
			$ENV{SystemDrive}/MySQL/*/include)
ENDIF (MYSQL_CONFIG_EXECUTABLE)

#----------------- FIND MYSQL_LIB_DIR -------------------
IF (MYSQL_CONFIG_EXECUTABLE)
    _MYSQL_CONFIG(MYSQL_LIBRARIES    "(^| )-l" "--libs")
    _MYSQL_CONFIG(MYSQL_LIB_DIR "(^| )-L" "--libs")
	FIND_LIBRARY(MYSQL_LIB NAMES mysqlclient HINTS ${MYSQL_LIB_DIR})
    GET_FILENAME_COMPONENT (MYSQL_LIB ${MYSQL_LIB} NAME)

ELSE (MYSQL_CONFIG_EXECUTABLE)
    FIND_LIBRARY(MYSQL_LIB NAMES mysqlclient
        	     ${WITH_MYSQL_LIBS}
        	     ${WITH_MYSQL_ROOT}/lib
        	     ${WITH_MYSQL_ROOT}/lib/mysql
                 PATHS
                 $ENV{MYSQL_DIR}/libmysql_r/.libs
                 $ENV{MYSQL_DIR}/lib
                 $ENV{MYSQL_DIR}/lib/mysql
                 /usr/lib/mysql
                 /usr/local/lib/mysql
                 /usr/local/mysql/lib
                 /usr/local/mysql/lib/mysql
                 /opt/mysql/mysql/lib
                 /opt/mysql/mysql/lib/mysql)
    SET(MYSQL_LIBRARIES mysqlclient )
    IF(MYSQL_LIB)
        GET_FILENAME_COMPONENT(MYSQL_LIB_DIR ${MYSQL_LIB} PATH)
        SET(MYSQL_LIBRARIES MYSQL_LIB)
		GET_FILENAME_COMPONENT(MYSQL_LIB ${MYSQL_LIB} NAME)
    ENDIF(MYSQL_LIB)
ENDIF (MYSQL_CONFIG_EXECUTABLE)

SET(VERBOSE 1)
IF (MYSQL_INCLUDE_DIR AND MYSQL_LIB_DIR)

	MESSAGE(STATUS "MySQL Include dir: ${MYSQL_INCLUDE_DIR}")
	MESSAGE(STATUS "MySQL Library    : ${MYSQL_LIBRARIES}")
	MESSAGE(STATUS "MySQL Library dir: ${MYSQL_LIB_DIR}")
	MESSAGE(STATUS "MySQL CXXFLAGS: ${MYSQL_CXXFLAGS}")
	MESSAGE(STATUS "MySQL Link flags: ${MYSQL_LINK_FLAGS}")
	MESSAGE(STATUS "MySQL Library Name   : ${MYSQL_LIB}")

	SET(MYSQL_FOUND TRUE)
ELSE (MYSQL_INCLUDE_DIR AND MYSQL_LIB_DIR)
	MESSAGE(SEND_ERROR "mysql_config wasn't found, -DMYSQL_CONFIG_EXECUTABLE=...")
	MESSAGE(FATAL_ERROR "Cannot find MySQL. Include dir: ${MYSQL_INCLUDE_DIR}  library dir: ${MYSQL_LIB_DIR} cxxflags: ${MYSQL_CXXFLAGS}")
ENDIF (MYSQL_INCLUDE_DIR AND MYSQL_LIB_DIR)
