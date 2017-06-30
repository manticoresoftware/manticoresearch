#=============================================================================
# Copyright 2017 Manticore Software Ltd.
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
# http://snowball.tartarus.org/dist/libstemmer_c.tgz

set ( STEMMER_URL "http://snowball.tartarus.org/dist/libstemmer_c.tgz" )
mark_as_advanced( STEMMER_URL)

find_package ( stemmer )
if ( STEMMER_FOUND )
	SET ( USE_LIBSTEMMER 1 )
	include_directories ( ${STEMMER_INCLUDE_DIR} )
	if ( STEMMER_INTERNAL )
		add_subdirectory ( ${CMAKE_SOURCE_DIR}/libstemmer_c )
		list ( APPEND EXTRA_LIBRARIES stemmer )
	else ()
		list ( APPEND EXTRA_LIBRARIES ${STEMMER_LIBRARY} )
	endif ( STEMMER_INTERNAL )
else ( STEMMER_FOUND )
	if ( EXISTS "${LIBS_BUNDLE}/libstemmer_c.tgz" )
		message ( STATUS "Unpack Stemmer from ${LIBS_BUNDLE}/libstemmer_c.tgz" )
		execute_process (
				COMMAND "${CMAKE_COMMAND}" -E tar xfz "${LIBS_BUNDLE}/libstemmer_c.tgz"
				WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}" )
		# download from github as zip archive
	else ( EXISTS "${LIBS_BUNDLE}/libstemmer_c.tgz" )
		set ( STEMMER_URL "http://snowball.tartarus.org/dist/libstemmer_c.tgz" )
		if ( NOT EXISTS "${MANTICORE_BINARY_DIR}/libstemmer_c.tgz" )
			message ( STATUS "Downloading Stemmer" )
			file ( DOWNLOAD ${STEMMER_URL} ${MANTICORE_BINARY_DIR}/libstemmer_c.tgz SHOW_PROGRESS )
		endif ()
		message ( STATUS "Unpack Stemmer from ${MANTICORE_BINARY_DIR}/libstemmer_c.tgz" )
		execute_process (
				COMMAND "${CMAKE_COMMAND}" -E tar xfz "${MANTICORE_BINARY_DIR}/libstemmer_c.tgz"
				WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}" )
	endif ( EXISTS "${LIBS_BUNDLE}/libstemmer_c.tgz" )
	set ( STEMMER_BASEDIR "${MANTICORE_BINARY_DIR}/libstemmer_c" )
	# copy our CMakeLists there
	if ( NOT EXISTS "${STEMMER_BASEDIR}/CMakeLists.txt" )
		configure_file ( "${CMAKE_SOURCE_DIR}/libstemmer_c/CMakeLists.txt" "${STEMMER_BASEDIR}/CMakeLists.txt" @ONLY )
	endif ()
	set ( USE_LIBSTEMMER 1 )
	set ( STEMMER_INCLUDE_DIR "${STEMMER_BASEDIR}/include" )
	include_directories ( ${STEMMER_BASEDIR}/include )
	add_subdirectory ( ${STEMMER_BASEDIR} ${STEMMER_BASEDIR} EXCLUDE_FROM_ALL )
	list ( APPEND EXTRA_LIBRARIES stemmer )
	if ( NOT EXISTS "${STEMMER_BASEDIR}/CMakeLists.txt" )
		message ( SEND_ERROR "missing libstemmer sources from libstemmer_c.
Please download the C version of libstemmer library from
http://snowball.tartarus.org/ and extract its sources over libstemmer_c/
subdirectory in order to build Sphinx with libstemmer support. Or
install the package named like 'libstemmer-dev' using your favorite
package manager." )
		unset ( WITH_STEMMER CACHE )
	endif ()
ENDIF ( STEMMER_FOUND )
