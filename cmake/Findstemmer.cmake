# - Find STEMMER library
# Find the native STEMMER includes and library
# This module defines
#  STEMMER_INCLUDE_DIR, where to find libstemmer.h, etc.
#  STEMMER_LIBRARIES, libraries to link against to use STEMMER.
#  STEMMER_FOUND, If false, do not try to use STEMMER.
# also defined, but not for general use are
#  STEMMER_LIBRARY, where to find the STEMMER library.

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

FIND_PATH(STEMMER_INCLUDE_DIR libstemmer.h)

SET(STEMMER_NAMES ${STEMMER_NAMES} stemmer stemmer_c)
FIND_LIBRARY(STEMMER_LIBRARY NAMES ${STEMMER_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set STEMMER_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(STEMMER  DEFAULT_MSG  STEMMER_LIBRARY  STEMMER_INCLUDE_DIR)

IF(STEMMER_FOUND)
  SET( STEMMER_LIBRARIES ${STEMMER_LIBRARY} )
ENDIF(STEMMER_FOUND)

MARK_AS_ADVANCED(STEMMER_INCLUDE_DIR STEMMER_LIBRARY)
