#.rst:
# FindRE2
# --------------
#
# Find RE2 library and headers
#
# The module defines the following variables:
#
# ::
#
#   RE2_FOUND        - true if libre2 was found
#   RE2_INCLUDE_DIRS - include search path
#   RE2_LIBRARIES    - libraries to link
# The module checks also these variables:
#   WITH_RE2_ROOT    - the full path to the libre2
#                       if so, it will have the highest priority to find
#   WITH_RE2_INCLUDES   - where to find the header files
#   WITH_RE2_LIBS       - where to search for the lib

#=============================================================================
# Copyright 2017-2021, Manticore Software LTD (https://manticoresearch.com)
# Copyright 2015-2016 Sphinx Technologies, Inc.
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

# we may have following variables from the top dir
#	set (RE2_INCLUDES "/usr/include;/usr/include/re2" CACHE PATH "path to re2 header files")
#	set (RE2_LIBRARIES "/usr/lib/x86_64-linux-gnu;/usr/lib64;/usr/local/lib64;/usr/lib/i386-linux-gnu;/usr/lib;/usr/local/lib" CACHE PATH "path to re2 libraries")

function(check_re2 HINT)
	if (RE2_LIBRARY AND NOT EXISTS ${RE2_LIBRARY})
		unset(RE2_LIBRARY CACHE)
	endif ()
	FIND_LIBRARY(RE2_LIBRARY NAMES re2 RE2 libre2.a HINTS ${HINT} NO_DEFAULT_PATH)
endfunction()

# First check if include path was explicitly given.
# If so, it has the maximum priority over any other possibilities
if ( EXISTS "${WITH_RE2_ROOT}/re2/re2.h" )
	check_re2 ("${WITH_RE2_ROOT}/obj/")
	set ( RE2_INCLUDE_DIRS ${WITH_RE2_ROOT} )

else ()
	# Check if there are any sources in ./libre2 path
	if ( EXISTS ${CMAKE_SOURCE_DIR}/libre2/re2/re2.h )
		check_re2("${CMAKE_SOURCE_DIR}/libre2/obj/")
		set ( RE2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/libre2" )

	else ()
		if ( EXISTS ${WITH_RE2_INCLUDES} )
			set ( RE2_INCLUDE_DIRS "${WITH_RE2_INCLUDES}" )
		else ()
			find_path ( RE2_INCLUDE_DIRS NAMES re2/re2.h PATHS /usr/include /usr/include/re2 )
		endif ()

		if ( WITH_RE2_LIBS )
			FIND_LIBRARY(RE2_LIBRARY NAMES re2 RE2 libre2.a
					PATHS
					${WITH_RE2_LIBS}
					/usr/lib/x86_64-linux-gnu
					/usr/lib64
					/usr/local/lib64
					/usr/lib/i386-linux-gnu
					/usr/lib
					/usr/local/lib)
		endif ()
	endif ()
endif ()

mark_as_advanced ( RE2_INCLUDE_DIRS RE2_LIBRARY )

# Handle the QUIETLY and REQUIRED arguments and set LIBICONV_FOUND
# to TRUE if all listed variables are TRUE.
# (Use ${CMAKE_ROOT}/Modules instead of ${CMAKE_CURRENT_LIST_DIR} because CMake
#  itself includes this FindLibArchive when built with an older CMake that does
#  not provide it.  The older CMake also does not have CMAKE_CURRENT_LIST_DIR.)
include ( ${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake )
find_package_handle_standard_args ( RE2 REQUIRED_VARS RE2_INCLUDE_DIRS RE2_LIBRARY )

if ( RE2_FOUND )
	set ( RE2_LIBRARIES ${RE2_LIBRARY} )
endif ()