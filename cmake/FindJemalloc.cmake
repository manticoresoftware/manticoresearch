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

if (PKG_CONFIG_FOUND)
	pkg_check_modules ( JEMALLOC QUIET jemalloc )
	implib_config ( JEMALLOC Jemalloc::Jemalloc )
endif ()

if (NOT JEMALLOC_FOUND )
	find_path ( JEMALLOC_INCLUDE_DIRS NAMES "jemalloc/jemalloc.h" )
	find_library ( JEMALLOC_LINK_LIBRARIES NAMES jemalloc libjemalloc NAMES_PER_DIR )
endif()

mark_as_advanced ( JEMALLOC_LINK_LIBRARIES JEMALLOC_INCLUDE_DIRS )

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args ( Jemalloc REQUIRED_VARS JEMALLOC_LINK_LIBRARIES )
set_package_properties (Jemalloc PROPERTIES TYPE RUNTIME
		DESCRIPTION "a general purpose malloc(3) implementation that emphasizes fragmentation avoidance and scalable concurrency support"
		URL "http://jemalloc.net/"
		)

include ( helpers )
implib_config ( JEMALLOC Jemalloc::Jemalloc )
