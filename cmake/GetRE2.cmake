#=============================================================================
# Copyright 2017-2025, Manticore Software LTD (https://manticoresearch.com)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# This file need to get RE2 library sources
# First it try 'traditional' way - find RE2 lib.
# Then (if it is not found) it try to look into ${LIBS_BUNDLE} for file named 're2-master.zip'
# It is supposed, that file (if any) contains github's master archive of RE2 sources.
# If no file found, it will  try to fetch it from github, address
# https://github.com/manticoresoftware/re2/archive/master.zip

set ( RE2_REPO "https://github.com/manticoresoftware/re2" )
set ( RE2_BRANCH "2015-06-01" ) # specific tag for reproducable builds
set ( RE2_SRC_MD5 "023053ef20051a0fc5911a76869be59f" )

set ( RE2_GITHUB "${RE2_REPO}/archive/${RE2_BRANCH}.zip" )
set ( RE2_BUNDLE "${LIBS_BUNDLE}/re2-${RE2_BRANCH}.zip" )

cmake_minimum_required ( VERSION 3.17 FATAL_ERROR )
include ( update_bundle )

# if it is allowed to use system library - try to use it
if (NOT WITH_RE2_FORCE_STATIC)
	find_package ( re2 MODULE QUIET )
	return_if_target_found ( re2::re2 "as default (sys or other) lib" )
endif ()

# determine destination folder where we expect pre-built re2
find_package ( re2 QUIET CONFIG )
return_if_target_found ( re2::re2 "found ready (no need to build)" )

function ( PATCH_GIT RESULT RE2_SRC )
	find_package ( Git QUIET )
	if (NOT GIT_EXECUTABLE)
		return ()
	endif ()
	execute_process ( COMMAND "${GIT_EXECUTABLE}" apply libre2.patch WORKING_DIRECTORY "${RE2_SRC}" )
	set ( ${RESULT} TRUE PARENT_SCOPE )
endfunction ()

function ( PATCH_PATCH RESULT RE2_SRC )
	find_program ( PATCH_PROG patch )
	if (NOT PATCH_PROG)
		return ()
	endif ()
	execute_process ( COMMAND "${PATCH_PROG}" -p1 --binary -i libre2.patch WORKING_DIRECTORY "${RE2_SRC}" )
	set ( ${RESULT} TRUE PARENT_SCOPE )
endfunction ()

# cb to finalize RE2 sources (patch, add cmake)
function ( PREPARE_RE2 RE2_SRC )
	# check if it is already patched before
	if (EXISTS "${RE2_SRC}/is_patched.txt")
		return ()
	endif ()

	file ( COPY "${MANTICORE_SOURCE_DIR}/libre2/libre2.patch" DESTINATION "${RE2_SRC}" )
	patch_git ( PATCHED ${RE2_SRC} )
	if (NOT PATCHED)
		patch_patch ( PATCHED ${RE2_SRC} )
	endif ()

	if (NOT PATCHED)
		message ( FATAL_ERROR "Couldn't patch RE2 distro. No Git or Patch found" )
		return ()
	endif ()

	file ( WRITE "${RE2_SRC}/is_patched.txt" "ok" )
	execute_process ( COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MANTICORE_SOURCE_DIR}/libre2/CMakeLists.txt" "${RE2_SRC}/CMakeLists.txt" )
endfunction ()

# prepare sources
select_nearest_url ( RE2_PLACE re2 ${RE2_BUNDLE} ${RE2_GITHUB} )
fetch_and_check ( re2 ${RE2_PLACE} ${RE2_SRC_MD5} RE2_SRC )
prepare_re2 ( ${RE2_SRC} )

# build
get_build ( RE2_BUILD re2 )
external_build ( re2 RE2_SRC RE2_BUILD )

# now it should find
find_package ( re2 REQUIRED CONFIG )
return_if_target_found ( re2::re2 "was built" )
