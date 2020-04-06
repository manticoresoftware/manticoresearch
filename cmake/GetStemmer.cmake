#=============================================================================
# Copyright 2017-2020, Manticore Software LTD (http://manticoresearch.com)
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

include(update_bundle)

set ( STEMMER_GITHUB "https://snowballstem.org/dist/libstemmer_c.tgz" )
set ( STEMMER_BUNDLEZIP "libstemmer_c.tgz")

# cb to finalize stemmer sources (add cmake)
function(PREPARE_STEMMER STEMMER_SRC)
	# check if it is already patched before
	if (NOT EXISTS "${STEMMER_SRC}/CMakeLists.txt")
		configure_file("${CMAKE_SOURCE_DIR}/libstemmer_c/CMakeLists.txt" "${STEMMER_SRC}/CMakeLists.txt" COPYONLY)
	endif ()
endfunction()

# cb to realize if we have in-source unpacked stemmer
function(CHECK_STEMMER_SRC RESULT HINT)
	if (HINT STREQUAL EMBEDDED AND EXISTS ${MANTICORE_SOURCE_DIR}/libstemmer_c/include/libstemmer.h)
		set(STEMMER_SRC "${MANTICORE_SOURCE_DIR}/libstemmer_c" PARENT_SCOPE)
		set(STEMMER_BUILD "${MANTICORE_SOURCE_DIR}/libstemmer_c" PARENT_SCOPE)
		set(${RESULT} TRUE PARENT_SCOPE)
	elseif(EXISTS ${HINT}/include/libstemmer.h)
		set(${RESULT} TRUE PARENT_SCOPE)
	endif()
endfunction()


provide(STEMMER "${STEMMER_GITHUB}" "${STEMMER_BUNDLEZIP}")
if (STEMMER_FROMSOURCES)
	add_subdirectory(${STEMMER_SRC} ${STEMMER_BUILD} EXCLUDE_FROM_ALL)
	set(STEMMER_LIBRARIES stemmer)
elseif (NOT STEMMER_FOUND)
	message(SEND_ERROR "missing libstemmer sources from libstemmer_c.
Please download the C version of libstemmer library from
https://snowballstem.org/ and extract its sources over libstemmer_c/
subdirectory in order to build Manticore with libstemmer support. Or
install the package named like 'libstemmer-dev' using your favorite
package manager.")
	unset(WITH_STEMMER CACHE)
	return()
endif ()

list(APPEND EXTRA_LIBRARIES ${STEMMER_LIBRARIES})
set(USE_LIBSTEMMER 1)

diag(STEMMER_FOUND)
diag(STEMMER_INCLUDE_DIRS)
diag(STEMMER_LIBRARIES)
diag(STEMMER_SRC)
diag(STEMMER_BUILD)
diag(STEMMER_FROMSOURCES)
