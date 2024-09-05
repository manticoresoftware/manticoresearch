# this cmake file fully replaces all old things like svnxrev, hgxrev, etc scripts.
cmake_minimum_required ( VERSION 3.17 )

# guess version strings from current git repo
function ( guess_from_git )

	if (NOT EXISTS "${MANTICORE_SOURCE_DIR}/.git")
		return ()
	endif ()

	find_package ( Git QUIET )
	if (NOT GIT_FOUND)
		return ()
	endif ()

	# without this in some environments you can get error "detected dubious ownership in repository"
	# `git config --global --add safe.directory '*'` in the docker image it runs in may not help. TODO: check why
  execute_process ( COMMAND "${GIT_EXECUTABLE}" config --global --add safe.directory "${MANTICORE_SOURCE_DIR}")

	# extract short has as SPH_GIT_COMMIT_ID
	execute_process ( COMMAND "${GIT_EXECUTABLE}" log -1 --format=%h
			WORKING_DIRECTORY "${MANTICORE_SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE SPH_GIT_COMMIT_ID
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE )
	set ( SPH_GIT_COMMIT_ID "${SPH_GIT_COMMIT_ID}" PARENT_SCOPE )

	# extract timestamp and make number YYMMDDHH from it
	execute_process ( COMMAND "${GIT_EXECUTABLE}" log -1 --date=format-local:"%y%m%d%H" --format=%cd
			WORKING_DIRECTORY "${MANTICORE_SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE GIT_TIMESTAMP_ID
			OUTPUT_STRIP_TRAILING_WHITESPACE )

	string ( SUBSTRING "${GIT_TIMESTAMP_ID}" 1 8 GIT_TIMESTAMP_ID )
	set ( GIT_TIMESTAMP_ID ${GIT_TIMESTAMP_ID} PARENT_SCOPE )

	# timestamp for reproducible packages
	execute_process ( COMMAND "${GIT_EXECUTABLE}" log -1 --pretty=%ct
			WORKING_DIRECTORY "${MANTICORE_SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE GIT_EPOCH_ID
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE )
	set ( SOURCE_DATE_EPOCH ${GIT_EPOCH_ID} PARENT_SCOPE )

	# extract branch name (top of 'git status -s -b'), throw out leading '## '
	execute_process ( COMMAND "${GIT_EXECUTABLE}" status -s -b
			WORKING_DIRECTORY "${MANTICORE_SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE GIT_BRANCH_ID
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE )
	string ( REGEX REPLACE "\n.*$" "" GIT_BRANCH_ID "${GIT_BRANCH_ID}" )
	string ( REPLACE "## " "" GIT_BRANCH_ID "${GIT_BRANCH_ID}" )
	set ( GIT_BRANCH_ID "git branch ${GIT_BRANCH_ID}" PARENT_SCOPE )
endfunction ()

# guess version strings from template header file (git archive mark it there)
function ( extract_from_git_slug HEADER )
	if (EXISTS "${HEADER}")
		FILE ( STRINGS "${HEADER}" _CONTENT )
		foreach (LINE ${_CONTENT})
			# match definitions like - // GIT_*_ID VALUE
			if ("${LINE}" MATCHES "^//[ \t]+(GIT_.*_ID)[ \t]\"(.*)\"")
				set ( ${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" )
			endif ()
		endforeach ()
		if (GIT_COMMIT_ID STREQUAL "$Format:%h$")
			return () # no slug
		endif ()
		# commit id
		set ( SPH_GIT_COMMIT_ID "${GIT_COMMIT_ID}" PARENT_SCOPE )
		# timestamp
		string ( REPLACE "-" "" GIT_TIMESTAMP_ID "${GIT_TIMESTAMP_ID}" )
		string ( SUBSTRING "${GIT_TIMESTAMP_ID}" 2 6 GIT_TIMESTAMP_ID )
		set ( GIT_TIMESTAMP_ID "${GIT_TIMESTAMP_ID}" PARENT_SCOPE )
		# epoch for packaging
		set ( SOURCE_DATE_EPOCH ${GIT_EPOCH_ID} PARENT_SCOPE )
		# branch id
		set ( GIT_BRANCH_ID "from tarball" PARENT_SCOPE )
	endif ()
endfunction ()

# function definitions finished, execution starts from here
##################################

# first try to use binary git
guess_from_git ()

# 2-nd try - if we build from git archive. Correct hash and date provided then, but no branch
if (NOT SPH_GIT_COMMIT_ID)
	extract_from_git_slug ( "${MANTICORE_SOURCE_DIR}/src/sphinxversion.h.in" )
endif ()

# extract version number string from sphinxversion.h.in
if (NOT VERNUMBERS)
	FILE ( STRINGS "${MANTICORE_SOURCE_DIR}/src/sphinxversion.h.in" _STRINGS LIMIT_COUNT 500
			REGEX "^#define[ \t]+VERNUMBERS.*" )
	STRING ( REGEX REPLACE ".*\"(.*)\"(.*)$" "\\1" VERNUMBERS "${_STRINGS}" )
endif ()

if (NOT SPH_GIT_COMMIT_ID)
	# nothing found, fall back to fake 'developer' version. That is ok if sources mirrored to another host without .git
	message ( STATUS "Dev mode, no guess, using predefined version" )
	if (NOT VERNUMBERS)
		set ( VERNUMBERS "0.0.1" )
	endif ()
	set ( GIT_TIMESTAMP_ID "220512" )
	set ( SPH_GIT_COMMIT_ID "DEADBEEF" )
	set ( BUILD_TAG "devmode" )
	set ( GIT_BRANCH_ID "developer version" )
	set ( SOURCE_DATE_EPOCH "1607089638" )
	set ( DEV_BUILD ON )
endif ()

set ( GDB_SOURCE_DIR "${MANTICORE_SOURCE_DIR}" )

# determine build tag as even/odd value of patch version
if (NOT BUILD_TAG)
	string ( REGEX MATCH "[0-9]+.[0-9]+.([0-9]+)" _ ${VERNUMBERS} )
	math ( EXPR oddvalue "${CMAKE_MATCH_1} % 2" OUTPUT_FORMAT DECIMAL )

	if (oddvalue)
		set ( BUILD_TAG "dev" )
		set ( DEV_BUILD ON )
	endif ()
endif ()

# All info collected (we need SPH_GIT_COMMIT_ID, GIT_TIMESTAMP_ID, GIT_BRANCH_ID and BUILD_TAG, if any)
set ( VERFILE "${MANTICORE_BINARY_DIR}/config/gen_sphinxversion.h" )

configure_file ( "${MANTICORE_SOURCE_DIR}/src/sphinxversion.h.in" "${VERFILE}" )
configure_file ( "${MANTICORE_SOURCE_DIR}/dist/CPackOptions.cmake.in" "${MANTICORE_BINARY_DIR}/config/CPackOptions.cmake" @ONLY )

# configure packaging
SET ( ENV{SOURCE_DATE_EPOCH} "${SOURCE_DATE_EPOCH}" ) # that makes builds reproducable
SET ( CPACK_PACKAGE_VERSION "${VERNUMBERS}-${GIT_TIMESTAMP_ID}-${SPH_GIT_COMMIT_ID}" )
string ( TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE )

SET ( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}-${CPACK_PACKAGE_VERSION}" )
SET ( CPACK_RPM_PACKAGE_VERSION "${VERNUMBERS}_${GIT_TIMESTAMP_ID}.${SPH_GIT_COMMIT_ID}" )
