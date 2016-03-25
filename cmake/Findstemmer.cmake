# - Find STEMMER library
# Find the native STEMMER includes and library
# This module defines
#  STEMMER_INCLUDE_DIR, where to find libstemmer.h, etc.
#  STEMMER_LIBRARIES, libraries to link against to use STEMMER.
#  STEMMER_FOUND, If false, do not try to use STEMMER.
#  STEMMER_INTERNAL, if true, stemmer need to be built especially
#=============================================================================
# Copyright 2014 Sphinx Technologies, Inc.
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

if (EXISTS ${CMAKE_SOURCE_DIR}/libstemmer_c/include/libstemmer.h)
	set (STEMMER_INTERNAL 1)
	MARK_AS_ADVANCED(STEMMER_INTERNAL)
	set (STEMMER_LIBRARY ${CMAKE_SOURCE_DIR}/libstemmer_c/libstemmer.a)
	set (STEMMER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libstemmer_c/include)
else()

	FIND_PATH(STEMMER_INCLUDE_DIR libstemmer.h)

	SET(STEMMER_NAMES ${STEMMER_NAMES} stemmer stemmer_c)
	FIND_LIBRARY(STEMMER_LIBRARY NAMES ${STEMMER_NAMES} )
endif()


# handle the QUIETLY and REQUIRED arguments and set STEMMER_FOUND to TRUE if 
# all listed variables are TRUE
include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(STEMMER REQUIRED_VARS STEMMER_LIBRARY STEMMER_INCLUDE_DIR)

MARK_AS_ADVANCED(STEMMER_INCLUDE_DIR STEMMER_LIBRARY)

if(STEMMER_FOUND)
	set(STEMMER_LIBRARIES    ${STEMMER_LIBRARY})
endif()
