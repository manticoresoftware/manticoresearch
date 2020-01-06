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

IF ( POLICY CMP0074 )
	CMAKE_POLICY ( SET CMP0074 NEW )
ENDIF ()

set ( ICU_LIBDIR "${MANTICORE_BINARY_DIR}/icu-bin" )
set ( ICU_SRC "${MANTICORE_BINARY_DIR}/icu" )
mark_as_advanced ( ICU_SRC ICU_LIBDIR )
set ( ICUZIP "icu4c-65_1-src.tgz" )

include ( FetchContent )
# check whether we have local copy (to not disturb network)
if ( LIBS_BUNDLE AND EXISTS "${LIBS_BUNDLE}/${ICUZIP}" )
	set ( ICU_URL "${LIBS_BUNDLE}/${ICUZIP}" )
	file ( SHA1 "${ICU_URL}" SHA1ICU )
	set ( ICU_URL_HASH "SHA1=${SHA1ICU}" )
	message ( STATUS "Using ICU from ${ICU_URL} with hash ${SHA1ICU}" )

	FetchContent_Declare ( icu
			SOURCE_DIR "${ICU_SRC}"
			BINARY_DIR "${ICU_LIBDIR}"
			URL "${ICU_URL}"
			URL_HASH ${ICU_URL_HASH}
			)
else ()
	set ( ICU_URL_GITHUB "https://github.com/unicode-org/icu/releases/download/release-65-1/${ICUZIP}" )
	message ( STATUS "Using ICU from ${ICU_URL_GITHUB}" )
	message ( STATUS "(you can download the file and save it as '${ICUZIP}' into ${LIBS_BUNDLE}/) " )
	FetchContent_Declare ( icu
			SOURCE_DIR "${ICU_SRC}"
			BINARY_DIR "${ICU_LIBDIR}"
			URL "${ICU_URL_GITHUB}"
			)
endif ()

FetchContent_GetProperties ( icu )
if ( NOT icu_POPULATED )
	FetchContent_Populate ( icu )
endif ()

mark_as_advanced ( FETCHCONTENT_FULLY_DISCONNECTED FETCHCONTENT_QUIET FETCHCONTENT_UPDATES_DISCONNECTED
		FETCHCONTENT_SOURCE_DIR_ICU FETCHCONTENT_UPDATES_DISCONNECTED_ICU )

if ( icu_POPULATED )
	set ( USE_ICU 1 )
	include_directories ( "${icu_SOURCE_DIR}/source/common" )
	if ( NOT EXISTS "${ICU_SRC}/CMakeLists.txt" )
		configure_file ( "${CMAKE_SOURCE_DIR}/libicu/CMakeLists.txt" "${ICU_SRC}/CMakeLists.txt" COPYONLY )
	endif ()
	add_subdirectory ( ${icu_SOURCE_DIR} ${icu_BINARY_DIR} )
	set ( ICU_INCLUDE_DIRS "${ICU_SRC}/source/common" )
	set ( ICU_LIBRARIES icu )
endif ()

if ( WIN32 )
	# FIXME! need a proper way to detect if CMAKE_INSTALL_DATADIR is not default
	if ( NOT CMAKE_INSTALL_DATADIR STREQUAL "share" )
		add_compile_definitions ( ICU_DATA_DIR="${CMAKE_INSTALL_DATADIR}/icu" )
	else()
		add_compile_definitions ( ICU_DATA_DIR="../share/icu" )
	endif()
else()
	add_compile_definitions ( ICU_DATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${PACKAGE_NAME}/icu" )
endif()

if ( USE_ICU )
	include_directories ( ${ICU_INCLUDE_DIRS} )
	list ( APPEND EXTRA_LIBRARIES ${ICU_LIBRARIES} )
endif()

memcfgvalues ( USE_ICU )
