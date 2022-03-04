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
# Copyright 2021-2022, Manticore Software LTD (https://manticoresearch.com)
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

find_package ( PkgConfig QUIET )
if (PKG_CONFIG_FOUND)
	pkg_check_modules ( Valgrind QUIET valgrind )
endif ()

if (NOT Valgrind_FOUND)
	find_path ( Valgrind_INCLUDE_DIRS valgrind/valgrind.h
		/usr/include
		/usr/local/include
		${VALGRIND_PREFIX}/include
		$ENV{VALGRIND_PREFIX}/include
		)
endif ()

mark_as_advanced ( Valgrind_INCLUDE_DIRS )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( Valgrind REQUIRED_VARS Valgrind_INCLUDE_DIRS )

implib_includes ( Valgrind Valgrind::Valgrind )
