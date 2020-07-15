#.rst:
# FindValgrind
# --------------
#
# Find Valgrind program and includes
#
# The module defines the following variables:
#
# ::
#
#   VALGRIND_FOUND        - true if valgrind was found
#   VALGRIND_INCLUDE_DIRS - include search path
#   VALGRIND_PROGRAM      - executable (valgrind is not library, nothing to link)

#=============================================================================
# Copyright 2020, Manticore Software LTD (http://manticoresearch.com)
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

# First check if include path was explicitly given.
# If so, it has the maximum priority over any other possibilities

find_path (VALGRIND_INCLUDE_DIR valgrind/valgrind.h
		/usr/include
		/usr/local/include
		${VALGRIND_PREFIX}/include
		$ENV{VALGRIND_PREFIX}/include
		)

find_program(VALGRIND_PROGRAM NAMES valgrind PATH
		/usr/bin
		/usr/local/bin
		${VALGRIND_PREFIX}/bin
		$ENV{VALGRIND_PREFIX}/bin
		)

mark_as_advanced (VALGRIND_PROGRAM VALGRIND_INCLUDE_DIR )
# Handle the QUIETLY and REQUIRED arguments and set VALGRIND_FOUND
# to TRUE if all listed variables are TRUE.
# (Use ${CMAKE_ROOT}/Modules instead of ${CMAKE_CURRENT_LIST_DIR} because CMake
#  itself includes this FindLibArchive when built with an older CMake that does
#  not provide it.  The older CMake also does not have CMAKE_CURRENT_LIST_DIR.)
include ( ${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake )
find_package_handle_standard_args ( Valgrind REQUIRED_VARS VALGRIND_INCLUDE_DIR )

if ( VALGRIND_FOUND )
	add_library(valgrind::valgrind INTERFACE IMPORTED)
	set_target_properties(valgrind::valgrind PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${VALGRIND_INCLUDE_DIR}")
	set ( HAVE_VALGRIND 1)
endif()