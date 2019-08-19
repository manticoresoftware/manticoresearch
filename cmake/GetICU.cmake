#=============================================================================
# Copyright 2017-2019, Manticore Software LTD (http://manticoresearch.com)
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
set ( ICUZIP "icu4c-64_2-src.tgz" )

#set ( ICU_DEBUG ON )

macro( try_icu )
set ( NEED_ICU_FROMSOURCES 0 )
find_package ( ICU COMPONENTS uc )
if ( ICU_FOUND )

	set ( USE_ICU 1 )
	# check whether we will build ICU from sources ourselves
	if ( WITH_ICU_FORCE_STATIC )
		set ( NEED_ICU_FROMSOURCES 1 )
		unset ( ICU_INCLUDE_DIRS )
		unset ( ICU_LIBRARIES )
		message ( STATUS "ICU as sys shared lib found, but need sources" )
	else ()
		message ( STATUS "ICU as sys shared library" )
		set ( ICU_IS_SHARED 1 )
		mark_as_advanced ( ICU_IS_SHARED )
	endif ()
else ()
	set ( NEED_ICU_FROMSOURCES 1 )
endif()
mark_as_advanced(ICU_DERB_EXECUTABLE ICU_GENBRK_EXECUTABLE ICU_GENCCODE_EXECUTABLE
		ICU_GENCFU_EXECUTABLE ICU_GENCMN_EXECUTABLE ICU_GENCNVAL_EXECUTABLE
		ICU_GENDICT_EXECUTABLE ICU_GENNORM2_EXECUTABLE ICU_GENRB_EXECUTABLE
		ICU_GENSPREP_EXECUTABLE ICU_ICU-CONFIG_EXECUTABLE ICU_ICUINFO_EXECUTABLE
		ICU_ICUPKG_EXECUTABLE ICU_INCLUDE_DIR ICU_MAKECONV_EXECUTABLE ICU_MAKEFILE_INC
		ICU_PKGDATA_EXECUTABLE ICU_PKGDATA_INC ICU_UCONV_EXECUTABLE)
endmacro()

set (STORED_ICU "${ICU_LIBRARIES}")
try_icu()
if ( NOT ICU_FOUND )
	if ( APPLE )
		set ( ICU_BREW /usr/local/opt/icu4c )
		if ( EXISTS ${ICU_BREW} )
			if ( NOT "$ENV{ICU_ROOT}" STREQUAL "${ICU_BREW}" )
				message ( "Will retry with 'ICU_ROOT=${ICU_BREW}'" )
				message ( "Use 'export ICU_ROOT=/path/to/icu/root' if you want to build with alternate ICU.")
				set ( ENV{ICU_ROOT} "${ICU_BREW}" )
				try_icu()
			endif ()
		else()
			message ( "You can use brew-provided ICU by installing it with `brew install icu4c`" )
			message ( "Then try to reconfigure and maybe provide env ICU_ROOT with the path to ICU folder." )
		endif ()
	endif ()
endif()

if ( NEED_ICU_FROMSOURCES )
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
	set ( ICU_URL_GITHUB "https://github.com/unicode-org/icu/releases/download/release-64-2/${ICUZIP}" )
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
endif ( NEED_ICU_FROMSOURCES )

if ( STORED_ICU AND NOT STORED_ICU STREQUAL "${ICU_LIBRARIES}" )
	message (STATUS "EXPERIMENTAL! Use your provided ${STORED_ICU} instead of found ${ICU_LIBRARIES}")
	set ( ICU_LIBRARIES "${STORED_ICU}" CACHE STRING "User-provided ICU library" FORCE )
endif()

if ( USE_ICU )
	include_directories ( ${ICU_INCLUDE_DIRS} )
	if ( NOT ICU_IS_SHARED )
		list ( APPEND EXTRA_LIBRARIES ${ICU_LIBRARIES} )
	endif()
endif()

CMAKE_DEPENDENT_OPTION ( DL_ICU "load ICU library dynamically" ON "ICU_FOUND;HAVE_DLOPEN;ICU_IS_SHARED;USE_ICU" OFF )

if ( DL_ICU )
	if ( NOT ICU_CPP_FUNCS )
		set ( ICU_FUNCS
				BreakIterator::createWordInstance
				utext_openUTF8
				utext_close
				Locale::getChinese
				u_errorName
				u_setDataDirectory )
		include ( helpers )
		GET_NAMES ( "${ICU_LIBRARIES}" ICU_FUNCS ICU_CPP_FUNCS ICU_RAW_FUNCS )
	endif()

	if ( NOT ICU_CPP_FUNCS )
		SET ( DL_ICU 0 )
		message (STATUS "Unable to extract functions from ICU lib, falling back to explicit linkage" )
	else()

		# set them into cache - to avoid run get_names in future.
		set ( ICU_FUNCS "${ICU_FUNCS}" CACHE STRING "" )
		set ( ICU_CPP_FUNCS "${ICU_CPP_FUNCS}" CACHE STRING "" )
		set ( ICU_RAW_FUNCS "${ICU_RAW_FUNCS}" CACHE STRING "" )
		mark_as_advanced ( ICU_FUNCS ICU_CPP_FUNCS ICU_RAW_FUNCS )

		list ( LENGTH ICU_FUNCS NFUNCS )
		math ( EXPR NFUNCS "${NFUNCS}-1" )
		# prepare inclusion clause
		SET (icui "${MANTICORE_BINARY_DIR}/config/ldicu.inl")
		file (WRITE "${icui}" "// This is generated file. Look into GetICU.cmake for template\n\n")
		foreach (_icuf RANGE ${NFUNCS})
			list ( GET ICU_FUNCS ${_icuf} _icuff )
			list ( GET ICU_CPP_FUNCS ${_icuf} _icufcpp )
			file ( APPEND "${icui}" "static decltype ( &${_icufcpp} ) imp_${_icuff} = nullptr;\n" )
		endforeach()

		# InitDynamicIcu ()
		file ( APPEND "${icui}" "\nbool InitDynamicIcu ()\n{\n\tconst char* sFuncs[] = {\n" )
		foreach ( _icuf RANGE ${NFUNCS} )
			list ( GET ICU_RAW_FUNCS ${_icuf} _icufr )
			file ( APPEND "${icui}" "\t\t\"${_icufr}\",\n" )
		endforeach ()
		file ( APPEND "${icui}" "\n\t};\n\n\tvoid** pFuncs[] = {\n" )
		foreach ( _icuf RANGE ${NFUNCS} )
			list ( GET ICU_FUNCS ${_icuf} _icuff )
			file ( APPEND "${icui}" "\t\t( void** ) &imp_${_icuff},\n" )
		endforeach ()
		file ( APPEND "${icui}" "\n\t};\n\n" )
		file ( APPEND "${icui}" "\tstatic CSphDynamicLibrary dLib ( ICU_LIB );\n" )
		file ( APPEND "${icui}" "\treturn dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void** ));\n" )
		file ( APPEND "${icui}" "}\n" )
	endif()
endif()

memcfgvalues ( ICU_IS_SHARED )

if ( DL_ICU )
	set ( DL_ICU 1 )
	GET_SONAME ( "${ICU_LIBRARIES}" ICU_LIB )
	message ( STATUS "ICU will be loaded dynamically in runtime as ${ICU_LIB}" )
	memcfgvalues ( DL_ICU ICU_LIB )
else ( DL_ICU )
	list ( APPEND EXTRA_LIBRARIES ${ICU_LIBRARIES} )
	message ( STATUS "ICU will be linked as ${ICU_LIBRARIES}" )
endif ()
