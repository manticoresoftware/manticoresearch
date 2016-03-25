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
#   RE2_PATH         - the path to the packet

# The module checks also these variables:
#   WITH_RE2_ROOT    - the full path to the libre2
#                       if so, it will have the highest priority to find
#   WITH_RE2_INCLUDES   - where to find the header files
#   WITH_RE2_LIBS       - where to search for the lib

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

# we may have following variables from the top dir
#	set (RE2_INCLUDES "/usr/include;/usr/include/re2" CACHE PATH "path to re2 header files")
#	set (RE2_LIBS "/usr/lib/x86_64-linux-gnu;/usr/lib64;/usr/local/lib64;/usr/lib/i386-linux-gnu;/usr/lib;/usr/local/lib" CACHE PATH "path to re2 libraries")

# First check if include path was explicitly given.
# If so, it has the maximum priority over any other possibilities
if (EXISTS "${WITH_RE2_ROOT}/re2/re2.h")
	set (RE2_INCLUDE_DIRS ${WITH_RE2_ROOT})
	set (RE2_LIBRARY ${WITH_RE2_ROOT}/obj/libre2.a)
	if (EXISTS "${RE2_INCLUDE_DIRS}/Makefile")
		set (RE2_PATH ${RE2_INCLUDE_DIRS})
	endif (EXISTS "${RE2_INCLUDE_DIRS}/Makefile")
else (EXISTS "${WITH_RE2_ROOT}/re2/re2.h")

# Check if there are any sources in ./libre2 path


	if (EXISTS ${CMAKE_SOURCE_DIR}/libre2/re2/re2.h)
		set(RE2_LIBRARY "${CMAKE_SOURCE_DIR}/libre2/obj/libre2.a")
		set(RE2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/libre2")
		set(RE2_PATH "${CMAKE_SOURCE_DIR}/libre2")
	else (EXISTS ${CMAKE_SOURCE_DIR}/libre2/re2/re2.h)

		if (EXISTS ${WITH_RE2_INCLUDES})
			set (RE2_INCLUDE_DIRS "${WITH_RE2_INCLUDES}")
		else (EXISTS ${WITH_RE2_INCLUDES})
			find_path(RE2_INCLUDE_DIRS NAMES re2/re2.h PATHS /usr/include /usr/include/re2)
		endif (EXISTS ${WITH_RE2_INCLUDES})

		if ( EXISTS ${WITH_RE2_LIBS} )
			set (RE2_LIBRARY ${WITH_RE2_LIBS})
		else ( EXISTS ${WITH_RE2_LIBS} )
			find_file(RE2_LIBRARY libre2.a
				/usr/lib/x86_64-linux-gnu
				/usr/lib64
				/usr/local/lib64
				/usr/lib/i386-linux-gnu
				/usr/lib
				/usr/local/lib
			)
		endif ( EXISTS ${WITH_RE2_LIBS} )
	endif (EXISTS ${CMAKE_SOURCE_DIR}/libre2/re2/re2.h)
endif (EXISTS "${WITH_RE2_ROOT}/re2/re2.h")

mark_as_advanced(RE2_INCLUDE_DIRS RE2_LIBRARY)

# Handle the QUIETLY and REQUIRED arguments and set LIBICONV_FOUND
# to TRUE if all listed variables are TRUE.
# (Use ${CMAKE_ROOT}/Modules instead of ${CMAKE_CURRENT_LIST_DIR} because CMake
#  itself includes this FindLibArchive when built with an older CMake that does
#  not provide it.  The older CMake also does not have CMAKE_CURRENT_LIST_DIR.)
include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(RE2 REQUIRED_VARS RE2_INCLUDE_DIRS RE2_LIBRARY)

if(RE2_FOUND)
	set(RE2_LIBRARIES    ${RE2_LIBRARY})
endif()