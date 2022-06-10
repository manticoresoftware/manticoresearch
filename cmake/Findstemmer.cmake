# - Find STEMMER library
# Find the native STEMMER includes and library
# This module defines
#  STEMMER_INCLUDE_DIR, where to find libstemmer.h, etc.
#  STEMMER_LIBRARIES, libraries to link against to use STEMMER.
#  STEMMER_FOUND, If false, do not try to use STEMMER.
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

FIND_PATH ( STEMMER_INCLUDE_DIR libstemmer.h ONLY_CMAKE_FIND_ROOT_PATH )

SET ( STEMMER_NAMES ${STEMMER_NAMES} stemmer stemmer_c )
FIND_LIBRARY ( STEMMER_LIBRARY NAMES ${STEMMER_NAMES} ONLY_CMAKE_FIND_ROOT_PATH )

# handle the QUIETLY and REQUIRED arguments and set STEMMER_FOUND to TRUE if 
# all listed variables are TRUE
include ( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS ( stemmer REQUIRED_VARS STEMMER_LIBRARY STEMMER_INCLUDE_DIR )

MARK_AS_ADVANCED ( STEMMER_INCLUDE_DIR STEMMER_LIBRARY )

if (STEMMER_FOUND AND NOT TARGET stemmer::stemmer)
	add_library ( stemmer::stemmer UNKNOWN IMPORTED )
	set_target_properties ( stemmer::stemmer PROPERTIES
			IMPORTED_LOCATION "${STEMMER_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${STEMMER_INCLUDE_DIR}"
			)
endif ()