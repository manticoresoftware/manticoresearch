#.rst:
# FindJEMALLOC
# --------------
#
# Find Jemalloc library
#
# The module defines the following variables:
#
# ::
#
#   Jemalloc_FOUND        - true if Jemalloc was found
#   In case it found, imported target Jemalloc::Jemalloc is defined to link with

#=============================================================================
# Copyright 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

find_package (PkgConfig QUIET)

pkg_check_modules (PC_JEMALLOC QUIET jemalloc)

# Look for the header file.
find_path (JEMALLOC_INCLUDE_DIR NAMES "jemalloc/jemalloc.h" HINTS ${PC_JEMALLOC_INCLUDEDIR})

# Look for the library.
find_library (JEMALLOC_LIBRARY NAMES jemalloc libjemalloc NAMES_PER_DIR HINTS ${PC_JEMALLOC_LIBDIR})

mark_as_advanced ( JEMALLOC_LIBRARY JEMALLOC_INCLUDE_DIR )

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args ( Jemalloc REQUIRED_VARS JEMALLOC_LIBRARY )
set_package_properties (Jemalloc PROPERTIES TYPE RUNTIME
		DESCRIPTION "a general purpose malloc(3) implementation that emphasizes fragmentation avoidance and scalable concurrency support"
		URL "http://jemalloc.net/"
		)

if (JEMALLOC_FOUND AND NOT TARGET Jemalloc::Jemalloc)
	add_library (Jemalloc::Jemalloc UNKNOWN IMPORTED)
	set_target_properties( Jemalloc::Jemalloc PROPERTIES IMPORTED_LOCATION "${JEMALLOC_LIBRARY}" )
	if (JEMALLOC_INCLUDE_DIR)
		target_include_directories(Jemalloc::Jemalloc INTERFACE "${JEMALLOC_INCLUDE_DIR}")
	endif()
endif ()
