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
# This file need to get RE2 library sources
# First it try 'traditional' way - find RE2 lib.
# Then (if it is not found) it try to look into ${LIBS_BUNDLE} for file named 'master.zip'
# It is supposed, that file (if any) contains github's master archive of RE2 sources.
# If no file found, it will  try to fetch it from github, address
# https://github.com/manticoresoftware/re2/archive/master.zip

set ( RE2_URL "https://github.com/manticoresoftware/re2/archive/master.zip" )

set ( WITH_RE2_INCLUDES "" CACHE PATH "path to re2 header files" )
set ( WITH_RE2_LIBS "" CACHE PATH "path to re2 libraries" )
set ( WITH_RE2_ROOT "" CACHE PATH "path to the libre2 bundle (where both header and library lives)" )
mark_as_advanced ( RE2_URL WITH_RE2_INCLUDES WITH_RE2_LIBS WITH_RE2_ROOT )

find_package ( RE2 )
if ( RE2_FOUND )
	set ( USE_RE2 1 )
	include_directories ( ${RE2_INCLUDE_DIRS} )
	# check whether we will build RE2 from sources ourselves
	if ( RE2_PATH )
		add_subdirectory ( ${RE2_PATH} )
		list ( APPEND EXTRA_LIBRARIES RE2 )
	else ( RE2_PATH )
		list ( APPEND EXTRA_LIBRARIES ${RE2_LIBRARIES} )
	endif ( RE2_PATH )
else ( RE2_FOUND )
	set ( RE2_BASEDIR "${MANTICORE_BINARY_DIR}/libre2" )
	mark_as_advanced ( RE2_BASEDIR )
	if ( NOT EXISTS "${RE2_BASEDIR}/Makefile" )
		# check whether we have local copy (to not disturb network)
		if ( EXISTS "${LIBS_BUNDLE}/master.zip" )
			message ( STATUS "Unpack RE2 from ${LIBS_BUNDLE}/master.zip" )
			execute_process (
					COMMAND "${CMAKE_COMMAND}" -E tar xfz "${LIBS_BUNDLE}/master.zip"
					WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}" )
			# download from github as zip archive
		else ( EXISTS "${LIBS_BUNDLE}/master.zip" )
			if ( NOT EXISTS "${MANTICORE_BINARY_DIR}/master.zip" )
				message ( STATUS "Downloading RE2" )
				file ( DOWNLOAD ${RE2_URL} ${MANTICORE_BINARY_DIR}/master.zip SHOW_PROGRESS )
			endif ()
			message ( STATUS "Unpack RE2 from ${MANTICORE_BINARY_DIR}/master.zip" )
			execute_process (
					COMMAND "${CMAKE_COMMAND}" -E tar xfz "${MANTICORE_BINARY_DIR}/master.zip"
					WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}" )
		endif ( EXISTS "${LIBS_BUNDLE}/master.zip" )
		file ( RENAME "${MANTICORE_BINARY_DIR}/re2-master" "${RE2_BASEDIR}" )

		# also backup original Makefile; it is important step!
		configure_file ( "${RE2_BASEDIR}/Makefile" "${RE2_BASEDIR}/MakefileOrig" COPYONLY )
	endif ( NOT EXISTS "${RE2_BASEDIR}/Makefile" )

	# RE2 sources found. Now patch them, if necessary
	if ( NOT EXISTS "${RE2_BASEDIR}/is_patched.txt" )
		set ( PATCH_FILE "${CMAKE_SOURCE_DIR}/libre2/libre2.patch" )

		mark_as_advanced ( PATCH_FILE )
		message ( STATUS "Patching RE2" )
		find_package ( Git QUIET )
		if ( GIT_EXECUTABLE )
			execute_process ( COMMAND "${GIT_EXECUTABLE}" apply ${PATCH_FILE}
					WORKING_DIRECTORY "${RE2_BASEDIR}"
					)
			file ( WRITE "${RE2_BASEDIR}/is_patched.txt" "ok" )
			# no git
		else ()
			find_program ( PATCH_PROG patch )
			if ( PATCH_PROG )
				mark_as_advanced ( PATCH_PROG )
				execute_process (
						COMMAND "${PATCH_PROG}" -p1 --binary -i "${PATCH_FILE}"
						WORKING_DIRECTORY "${RE2_BASEDIR}"
				)
				file ( WRITE "${RE2_BASEDIR}/is_patched.txt" "ok" )
			endif ()
		endif ()
	endif ()
	if ( NOT EXISTS "${RE2_BASEDIR}/is_patched.txt" )
		message ( ERROR "Couldn't patch RE2 distro. No Git or Patch found" )
	endif ()

	# copy our CMakeLists there
	if ( NOT EXISTS "${RE2_BASEDIR}/CMakeLists.txt" )
		message ( STATUS "${CMAKE_SOURCE_DIR} - source dir" )
		configure_file ( "${CMAKE_SOURCE_DIR}/libre2/CMakeLists.txt" "${RE2_BASEDIR}/CMakeLists.txt" COPYONLY )
	endif ()
	set ( USE_RE2 1 )
	set ( RE2_INCLUDE_DIRS "${RE2_BASEDIR}" )
	include_directories ( ${RE2_BASEDIR} )
	add_subdirectory ( ${RE2_BASEDIR} ${RE2_BASEDIR} EXCLUDE_FROM_ALL )
	list ( APPEND EXTRA_LIBRARIES RE2 )
endif ( RE2_FOUND )
