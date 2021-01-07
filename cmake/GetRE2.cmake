#=============================================================================
# Copyright 2017-2021, Manticore Software LTD (https://manticoresearch.com)
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

#set(RE2_GITHUB "https://github.com/manticoresoftware/re2/archive/master.zip")
# download specific revision by tag
set(RE2_GITHUB "https://github.com/manticoresoftware/re2/archive/2015-06-01.zip")
set(RE2_BUNDLEZIP "re2-master.zip")

include(update_bundle)

set ( WITH_RE2_INCLUDES "" CACHE PATH "path to re2 header files" )
set ( WITH_RE2_LIBS "" CACHE PATH "path to re2 libraries" )
set ( WITH_RE2_ROOT "" CACHE PATH "path to the libre2 bundle (where both header and library lives)" )
mark_as_advanced ( WITH_RE2_INCLUDES WITH_RE2_LIBS WITH_RE2_ROOT )

function (PATCH_GIT RESULT RE2_SRC)
	find_package(Git QUIET)
	if (NOT GIT_EXECUTABLE)
		return()
	endif()
	execute_process(COMMAND "${GIT_EXECUTABLE}" apply libre2.patch WORKING_DIRECTORY "${RE2_SRC}" )
	set(${RESULT} TRUE PARENT_SCOPE)
endfunction()

function(PATCH_PATCH RESULT RE2_SRC)
	find_program(PATCH_PROG patch)
	if (NOT PATCH_PROG)
		return()
	endif ()
	execute_process( COMMAND "${PATCH_PROG}" -p1 --binary -i libre2.patch WORKING_DIRECTORY "${RE2_SRC}" )
	set(${RESULT} TRUE PARENT_SCOPE)
endfunction()

# cb to finalize RE2 sources (patch, add cmake)
function (PREPARE_RE2 RE2_SRC)
	# check if it is already patched before
	if (EXISTS "${RE2_SRC}/is_patched.txt")
		return()
	endif ()

	if (NOT EXISTS "${RE2_SRC}/MakefileOrig")
		configure_file("${RE2_SRC}/Makefile" "${RE2_SRC}/MakefileOrig" COPYONLY)
	endif ()

	file(COPY "${MANTICORE_SOURCE_DIR}/libre2/libre2.patch" DESTINATION "${RE2_SRC}")
	patch_git(PATCHED ${RE2_SRC})
	if (NOT PATCHED)
		patch_patch(PATCHED ${RE2_SRC})
	endif()

	if (NOT PATCHED)
		message(FATAL_ERROR "Couldn't patch RE2 distro. No Git or Patch found")
		return()
	endif()

	file(WRITE "${RE2_SRC}/is_patched.txt" "ok")
	configure_file("${MANTICORE_SOURCE_DIR}/libre2/CMakeLists.txt" "${RE2_SRC}/CMakeLists.txt" COPYONLY)
endfunction()

# cb to realize if we have in-source unpacked RE2
function(CHECK_RE2_SRC RESULT HINT)
	if (HINT STREQUAL EMBEDDED AND EXISTS "${MANTICORE_SOURCE_DIR}/libre2/re2/re2.h")
		set(RE2_SRC "${MANTICORE_SOURCE_DIR}/libre2" PARENT_SCOPE)
		set(RE2_BUILD "${MANTICORE_SOURCE_DIR}/libre2" PARENT_SCOPE)
		set(${RESULT} TRUE PARENT_SCOPE)
	elseif(EXISTS "${HINT}/re2/re2.h")
		set(${RESULT} TRUE PARENT_SCOPE)
	endif()
endfunction()

provide(RE2 "${RE2_GITHUB}" "${RE2_BUNDLEZIP}")
if (RE2_FROMSOURCES)
	add_subdirectory(${RE2_SRC} ${RE2_BUILD} EXCLUDE_FROM_ALL)
	set(RE2_LIBRARIES re2)
elseif (NOT RE2_FOUND)
	return()
endif()

list(APPEND EXTRA_LIBRARIES ${RE2_LIBRARIES})
set(USE_RE2 1)
memcfgvalues(USE_RE2)

message(STATUS "library: ${RE2_LIBRARIES}")
diag(RE2_FOUND)
diag(RE2_INCLUDE_DIRS)
diag(RE2_LIBRARIES)
diag(RE2_SRC)
diag(RE2_BUILD)
diag(RE2_FROMSOURCES)