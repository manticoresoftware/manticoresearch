#.rst:
# FindODBC
# --------------
#
# Find unixodbc library and headers
#
# The module defines the following variables:
#
# ::
#
#   ODBC_FOUND        - true if libarchive was found
#   ODBC_INCLUDE_DIRS - include search path
#   ODBC_LIBRARIES    - libraries to link

#=============================================================================
# Copyright 2015 Sphinx Technologies, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path ( ODBC_INCLUDE_DIRS sql.h )
find_library ( ODBC_LIBRARY NAMES odbc iodbc unixodbc )
mark_as_advanced ( ODBC_INCLUDE_DIRS ODBC_LIBRARY )

message ( STATUS "ODBC_INCLUDE_DIRS is ${ODBC_INCLUDE_DIRS}" )
message ( STATUS "ODBC_LIBRARY is ${ODBC_LIBRARY}" )

# Handle the QUIETLY and REQUIRED arguments and set LIBICONV_FOUND
# to TRUE if all listed variables are TRUE.
# (Use ${CMAKE_ROOT}/Modules instead of ${CMAKE_CURRENT_LIST_DIR} because CMake
#  itself includes this FindLibArchive when built with an older CMake that does
#  not provide it.  The older CMake also does not have CMAKE_CURRENT_LIST_DIR.)
include ( ${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake )
find_package_handle_standard_args ( ODBC REQUIRED_VARS ODBC_INCLUDE_DIRS ODBC_LIBRARY )

if ( ODBC_FOUND )
	set ( ODBC_LIBRARIES ${ODBC_LIBRARY} )
endif ()