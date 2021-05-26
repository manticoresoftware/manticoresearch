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
# imported interface target Valgrind::Valgrind with include also created
#
#=============================================================================
# Copyright 2021, Manticore Software LTD (https://manticoresearch.com)
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

mark_as_advanced (VALGRIND_INCLUDE_DIR)

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args (Valgrind REQUIRED_VARS VALGRIND_INCLUDE_DIR)

if (Valgrind_FOUND AND NOT TARGET Valgrind::Valgrind)
	add_library(Valgrind::Valgrind INTERFACE IMPORTED)
	set_target_properties(Valgrind::Valgrind PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${VALGRIND_INCLUDE_DIR}")
endif()