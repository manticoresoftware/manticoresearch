#.rst:
# FindJEMALLOC
# --------------
#
# Find JEMALLOC library and headers
#
# The module defines the following variables:
#
# ::
#
#   JEMALLOC_FOUND        - true if libre2 was found
#   JEMALLOC_INCLUDE_DIRS - include search path
#   JEMALLOC_LIBRARIES    - libraries to link

#=============================================================================
# Copyright 2017-2020, Manticore Software LTD (http://manticoresearch.com)
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

find_program ( JEMALLOC_CONFIG_PROGRAM jemalloc-config	)
if ( JEMALLOC_CONFIG_PROGRAM )
	# use jemalloc-config to set different vars
	execute_process (
			COMMAND ${JEMALLOC_CONFIG_PROGRAM} --libs
			OUTPUT_VARIABLE JEMALLOC_LIBS
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	execute_process (
			COMMAND ${JEMALLOC_CONFIG_PROGRAM} --libdir
			OUTPUT_VARIABLE JEMALLOC_LIBDIR
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	execute_process (
			COMMAND ${JEMALLOC_CONFIG_PROGRAM} --includedir
			OUTPUT_VARIABLE JEMALLOC_INCLUDE_DIRS
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	mark_as_advanced ( JEMALLOC_LIBS JEMALLOC_LIBDIR )
endif()
find_library ( JEMALLOC_LIBRARY jemalloc HINT "${JEMALLOC_LIBDIR}" )
mark_as_advanced ( JEMALLOC_LIBRARY JEMALLOC_CONFIG_PROGRAM )

# Handle the QUIETLY and REQUIRED arguments and set LIBICONV_FOUND
# to TRUE if all listed variables are TRUE.
# (Use ${CMAKE_ROOT}/Modules instead of ${CMAKE_CURRENT_LIST_DIR} because CMake
#  itself includes this FindLibArchive when built with an older CMake that does
#  not provide it.  The older CMake also does not have CMAKE_CURRENT_LIST_DIR.)
include ( ${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake )
find_package_handle_standard_args ( Jemalloc REQUIRED_VARS JEMALLOC_LIBRARY )

if ( JEMALLOC_FOUND )
	set ( JEMALLOC_LIBRARIES ${JEMALLOC_LIBRARY} )
endif ()