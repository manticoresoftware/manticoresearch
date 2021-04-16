# this cmake file fully replaces all old things like svnxrev, hgxrev, etc scripts.
cmake_minimum_required ( VERSION 2.8.12 )

# guess version strings from current git repo
function(guess_from_git)

	if (NOT EXISTS "${SOURCE_DIR}/.git")
		return()
	endif ()

	find_package(Git QUIET)
	if (NOT GIT_FOUND)
		return()
	endif()

	# extract short has as SPH_GIT_COMMIT_ID
	execute_process(COMMAND "${GIT_EXECUTABLE}" log -1 --format=%h
			WORKING_DIRECTORY "${SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE SPH_GIT_COMMIT_ID
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(SPH_GIT_COMMIT_ID "${SPH_GIT_COMMIT_ID}" PARENT_SCOPE)

	# extract timestamp and make number YYMMDD from it
	# it would be --date=format:%y%m%d, but old git on centos doesn't understand it
	execute_process(COMMAND "${GIT_EXECUTABLE}" log -1 --date=short --format=%cd
			WORKING_DIRECTORY "${SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE GIT_TIMESTAMP_ID
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REPLACE "-" "" GIT_TIMESTAMP_ID "${GIT_TIMESTAMP_ID}")
	string(SUBSTRING "${GIT_TIMESTAMP_ID}" 2 -1 GIT_TIMESTAMP_ID)
	set(GIT_TIMESTAMP_ID "${GIT_TIMESTAMP_ID}" PARENT_SCOPE)

	# timestamp for reproducable packages
	execute_process(COMMAND "${GIT_EXECUTABLE}" log -1 --pretty=%ct
			WORKING_DIRECTORY "${SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE GIT_EPOCH_ID
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(SOURCE_DATE_EPOCH ${GIT_EPOCH_ID} PARENT_SCOPE)

	# extract branch name (top of 'git status -s -b'), throw out leading '## '
	execute_process(COMMAND "${GIT_EXECUTABLE}" status -s -b
			WORKING_DIRECTORY "${SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE GIT_BRANCH_ID
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REGEX REPLACE "\n.*$" "" GIT_BRANCH_ID "${GIT_BRANCH_ID}")
	string(REPLACE "## " "" GIT_BRANCH_ID "${GIT_BRANCH_ID}")
	set(GIT_BRANCH_ID "git branch ${GIT_BRANCH_ID}" PARENT_SCOPE)
endfunction()

# guess version strings from template header file (git archive mark it there)
function(extract_from_git_slug HEADER)
	if (EXISTS "${HEADER}")
		FILE(READ "${HEADER}" _CONTENT)
		# replace lf into ';' (it makes list from the line)
		STRING(REGEX REPLACE "\n" ";" _CONTENT "${_CONTENT}")
		foreach (LINE ${_CONTENT})
			# match definitions like - // GIT_*_ID VALUE
			if ("${LINE}" MATCHES "^//[ \t]+(GIT_.*_ID)[ \t]\"(.*)\"")
				set(${CMAKE_MATCH_1} "${CMAKE_MATCH_2}")
			endif ()
		endforeach ()
		if (GIT_COMMIT_ID STREQUAL "$Format:%h$")
			return() # no slug
		endif()
		# commit id
		set(SPH_GIT_COMMIT_ID "${GIT_COMMIT_ID}" PARENT_SCOPE)
		# timestamp
		string(REPLACE "-" "" GIT_TIMESTAMP_ID "${GIT_TIMESTAMP_ID}")
		string(SUBSTRING "${GIT_TIMESTAMP_ID}" 2 6 GIT_TIMESTAMP_ID)
		set(GIT_TIMESTAMP_ID "${GIT_TIMESTAMP_ID}" PARENT_SCOPE)
		# epoch for packaging
		set(SOURCE_DATE_EPOCH ${GIT_EPOCH_ID} PARENT_SCOPE)
		# branch id
		set(GIT_BRANCH_ID "from tarball" PARENT_SCOPE)
	endif ()
endfunction()

# guess version strings from header file (may be pre-generated)
function (extract_from_header HEADER )
	if (EXISTS "${HEADER}")
		FILE(READ "${HEADER}" _CONTENT)
		# replace lf into ';' (it makes list from the line)
		STRING(REGEX REPLACE "\n" ";" _CONTENT "${_CONTENT}")
		foreach (LINE ${_CONTENT})
			# match definitions like - #define NAME "VALUE"
			IF ("${LINE}" MATCHES "^#define[ \t]+(.*)[ \t]+\"(.*)\"")
				set(${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" PARENT_SCOPE)
			endif ()
		endforeach ()
		STRING(TIMESTAMP GIT_EPOCH_ID "%s")
		set(SOURCE_DATE_EPOCH ${GIT_EPOCH_ID} PARENT_SCOPE)
		set(GIT_BRANCH_ID "from tarball" PARENT_SCOPE)
	endif()
endfunction()

# guess version strings from the name of the folder (if unpacked from tarball)
function(guess_from_dir_name SOURCE_DIR)
	get_filename_component(DIR ${SOURCE_DIR} NAME)
	if (NOT "${DIR}" MATCHES "^manticore-([0-9]+\\.[0-9]+\\.[0-9]+)-([0-9][0-9][0-9][0-9][0-9][0-9])-([0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]+)-?(.*)?$")
		return()
	endif ()
	set(VERNUMBERS "${CMAKE_MATCH_1}" PARENT_SCOPE)
	set(GIT_TIMESTAMP_ID "${CMAKE_MATCH_2}" PARENT_SCOPE)
	set(SPH_GIT_COMMIT_ID "${CMAKE_MATCH_3}" PARENT_SCOPE)
	set(BUILD_TAG "${CMAKE_MATCH_4}" PARENT_SCOPE)
	set(GIT_BRANCH_ID "from the folder '${DIR}'" PARENT_SCOPE)
	STRING(TIMESTAMP GIT_EPOCH_ID "%s")
	set(SOURCE_DATE_EPOCH ${GIT_EPOCH_ID} PARENT_SCOPE)
endfunction()

# function definitions finished, execution starts from here
##################################

# first try to use binary git
guess_from_git()

# 2-nd try - if we build from git archive. Correct hash and date provided then, but no branch
if (NOT SPH_GIT_COMMIT_ID)
	extract_from_git_slug("${SOURCE_DIR}/src/sphinxversion.h.in")
endif ()

# following are secondary legacy guesses
if ( NOT SPH_GIT_COMMIT_ID )
	extract_from_header("${SOURCE_DIR}/src/sphinxversion.h")
endif()

if ( NOT SPH_GIT_COMMIT_ID )
	guess_from_dir_name ("${SOURCE_DIR}")
endif ()

if (NOT SPH_GIT_COMMIT_ID)
	set(DEVMODE "$ENV{DEVMODE}")
	if ( DEVMODE )
		message(STATUS "Dev mode, no guess, using predefined version")
		set(VERNUMBERS "0.0.1")
		set(GIT_TIMESTAMP_ID "202020")
		set(SPH_GIT_COMMIT_ID "DEADBEEF")
		set(BUILD_TAG "devmode")
		set(GIT_BRANCH_ID "fake head from devmode")
		set(SOURCE_DATE_EPOCH "1607089638")
	endif ()
endif ()

# nothing found, bail with error
if ( NOT SPH_GIT_COMMIT_ID )
	message(FATAL_ERROR "Git not found, or the sources are not git clone, or not located in the folder originally unpacked from tarball, or not contain pre-created sphinxversion.h header. Please, put this file to your src/ folder manually. Devmode=${DEVMODE}")
endif ()

# extract version number string from sphinxversion.h.in
if ( NOT VERNUMBERS )
	FILE ( STRINGS "${SOURCE_DIR}/src/sphinxversion.h.in" _STRINGS LIMIT_COUNT 500
			REGEX "^#define[ \t]+VERNUMBERS.*" )
	STRING ( REGEX REPLACE ".*\"(.*)\"(.*)$" "\\1" VERNUMBERS "${_STRINGS}" )
endif()

set ( GDB_SOURCE_DIR "${SOURCE_DIR}" )

# All info collected (we need SPH_GIT_COMMIT_ID, GIT_TIMESTAMP_ID, GIT_BRANCH_ID and BUILD_TAG, if any)
set ( VERFILE "${BINARY_DIR}/config/gen_sphinxversion.h" )

configure_file ( "${SOURCE_DIR}/src/sphinxversion.h.in" "${VERFILE}1" )
file ( MD5 "${VERFILE}1" VERNEW )
set ( NEED_NEWFILE TRUE )

if ( EXISTS "${VERFILE}" )
	file ( MD5 "${VERFILE}" VEROLD )
	if ( VEROLD STREQUAL VERNEW )
		set ( NEED_NEWFILE FALSE )
	endif()
endif()

if ( NEED_NEWFILE )
	message ( STATUS "Version ${VERNUMBERS} ${SPH_GIT_COMMIT_ID}@${GIT_TIMESTAMP_ID}, ${GIT_BRANCH_ID}" )
	configure_file ( "${VERFILE}1" "${VERFILE}" COPYONLY )
	file ( REMOVE "${VERFILE}1" )
	configure_file("${SOURCE_DIR}/dist/CPackOptions.cmake.in" "${BINARY_DIR}/config/CPackOptions.cmake" @ONLY)
else()
	message ( STATUS "Version not changed: ${VERNUMBERS} ${SPH_GIT_COMMIT_ID}@${GIT_TIMESTAMP_ID}, ${GIT_BRANCH_ID}" )
endif()
