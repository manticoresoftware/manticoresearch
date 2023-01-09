#=============================================================================
# Copyright 2017-2023, Manticore Software LTD (https://manticoresearch.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# This file need to get libstemmer sources
# First it try 'traditional' way - find stemmer package.
# Then (if it is not found) it try to look into ${LIBS_BUNDLE} for file named 'libstemmer_c.tgz'
# It is supposed, that file (if any) contains archive from snowball with stemmer's sources.
# If no file found, it will try to fetch it from
# https://snowballstem.org/dist/libstemmer_c.tgz

set ( STEMMER_REMOTE "https://snowballstem.org/dist/libstemmer_c.tgz" )
set ( STEMMER_BUNDLEZIP "${LIBS_BUNDLE}/libstemmer_c.tgz" )
set ( STEMMER_SRC_MD5 "f8288a861db7c97dc4750020c7c7aa6f" )

cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )
include ( update_bundle )

# if it is allowed to use system library - try to use it
if (NOT WITH_STEMMER_FORCE_STATIC)
	find_package ( stemmer MODULE QUIET )
	return_if_target_found ( stemmer::stemmer "as default (sys or other)" )
endif ()

# determine destination folder where we expect pre-built stemmer
find_package ( stemmer CONFIG )
return_if_target_found ( stemmer::stemmer "ready (no need to build)" )

# not found. Populate and prepare sources
select_nearest_url ( STEMMER_PLACE stemmer ${STEMMER_BUNDLEZIP} ${STEMMER_REMOTE} )
fetch_and_check ( stemmer ${STEMMER_PLACE} ${STEMMER_SRC_MD5} STEMMER_SRC )
execute_process ( COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MANTICORE_SOURCE_DIR}/libstemmer_c/CMakeLists.txt" "${STEMMER_SRC}/CMakeLists.txt" )

# build external project
get_build ( STEMMER_BUILD stemmer )
external_build ( stemmer STEMMER_SRC STEMMER_BUILD )

# now it should find
find_package ( stemmer REQUIRED CONFIG )
return_if_target_found ( stemmer::stemmer "was built and saved" )
