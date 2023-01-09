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
set ( ICU_GITHUB "https://github.com/unicode-org/icu/releases/download/release-65-1/icu4c-65_1-src.tgz" )
set ( ICU_BUNDLE "${LIBS_BUNDLE}/icu4c-65_1-src.tgz" )
set ( ICU_SRC_MD5 "d1ff436e26cabcb28e6cb383d32d1339" )

cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )
include ( update_bundle )

# helpers
function ( install_icudata DEST )
	if (NOT TARGET icu::icudata)
		return ()
	endif ()
	get_target_property ( ICU_DATA icu::icudata INTERFACE_SOURCES )
	diag ( ICU_DATA )
	install ( FILES ${ICU_DATA} DESTINATION "${DEST}" COMPONENT icudata )
endfunction ()

# if it is allowed to use system library - try to use it
if (NOT WITH_ICU_FORCE_STATIC)
	find_package ( ICU MODULE QUIET COMPONENTS uc )
	if (ICU_UC_FOUND)
		add_library ( icu::icu ALIAS ICU::uc )
	endif ()
	return_if_target_found ( icu::icu "as default (sys or other)" )
endif ()

if (WITH_ICU_FORCE_BUILD) # special case for static binary
	select_nearest_url ( ICU_PLACE icu ${ICU_BUNDLE} ${ICU_GITHUB} )
	fetch_and_check ( icu ${ICU_PLACE} ${ICU_SRC_MD5} ICU_SRC )
	configure_file ( "${MANTICORE_SOURCE_DIR}/libicu/CMakeLists.txt" "${ICU_SRC}/CMakeLists.txt" COPYONLY )
	message ( STATUS "Build icu from sources since we want special build for static release" )
	set ( ICU_BUILD "${MANTICORE_BINARY_DIR}/icu-build-static-force" )
	external_build ( icu ICU_SRC ICU_BUILD "STATIC_BUILD=ON" )
	find_package ( icu REQUIRED CONFIG PATHS "${ICU_BUILD}" )
	trace ( icu::icu )
	return_if_target_found ( icu::icu "was built and saved" )
endif ()

# determine destination folder where we expect pre-built icu
find_package ( icu QUIET CONFIG )
return_if_target_found ( icu::icu "ready (no need to build)" )

# not found. Populate and prepare sources
select_nearest_url ( ICU_PLACE icu ${ICU_BUNDLE} ${ICU_GITHUB} )
fetch_and_check ( icu ${ICU_PLACE} ${ICU_SRC_MD5} ICU_SRC )
execute_process ( COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MANTICORE_SOURCE_DIR}/libicu/CMakeLists.txt" "${ICU_SRC}/CMakeLists.txt" )

# build external project
get_build ( ICU_BUILD icu )
external_build ( icu ICU_SRC ICU_BUILD )

# now it should find
find_package ( icu REQUIRED CONFIG )
return_if_target_found ( icu::icu "was built and saved" )
