# this cmake file fully replaces all old things like svnxrev, hgxrev, etc scripts.
cmake_minimum_required ( VERSION 2.8 )
if ( CMAKE_VERSION VERSION_GREATER "3.1" )
	CMAKE_POLICY ( SET CMP0053 OLD )
endif ()

# first try to use binary git
if ( EXISTS "${SOURCE_DIR}/.git" )
	find_package ( Git )
	if ( GIT_FOUND )
		execute_process ( COMMAND "${GIT_EXECUTABLE}" log -1 --format=%h
				WORKING_DIRECTORY "${SOURCE_DIR}"
				RESULT_VARIABLE res
				OUTPUT_VARIABLE SPH_GIT_COMMIT_ID
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE )
		execute_process ( COMMAND "${GIT_EXECUTABLE}" log -1 --date=format:%y%m%d --format=%ad
				WORKING_DIRECTORY "${SOURCE_DIR}"
				RESULT_VARIABLE res
				OUTPUT_VARIABLE GIT_TIMESTAMP_ID
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE )
		execute_process ( COMMAND "${GIT_EXECUTABLE}" status -s -b
				WORKING_DIRECTORY "${SOURCE_DIR}"
				RESULT_VARIABLE res
				OUTPUT_VARIABLE GIT_BRANCH_ID
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE )
		string ( REGEX REPLACE "\n.*$" "" GIT_BRANCH_ID "${GIT_BRANCH_ID}" )
		string ( REPLACE "## " "" GIT_BRANCH_ID "${GIT_BRANCH_ID}" )
	endif ( GIT_FOUND )
endif ()

# Notning found. Try back task - suppose se have pre-generated sphinxversion. Parse it.
if ( NOT SPH_GIT_COMMIT_ID )
	if ( EXISTS "${SOURCE_DIR}/src/sphinxversion.h" )
		FILE ( READ "${SOURCE_DIR}/src/sphinxversion.h" _CONTENT )
		# replace lf into ';' (it makes list from the line)
		STRING ( REGEX REPLACE "\n" ";" _CONTENT "${_CONTENT}" )
		foreach ( LINE ${_CONTENT} )
			# match definitions like - #define NAME "VALUE"
			IF ( "${LINE}" MATCHES "^#define[ \t]+(.*)[ \t]+\"(.*)\"" )
				set ( ${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" )
			endif ()
		endforeach ()
	else ()
		message ( SEND_ERROR "Git not found, or the sources are not git clone, or not contain pre-created sphinxversion.h header. Please, put this file to your src/ folder manually" )
	endif ()
endif ()

# All info collected (we need SPH_GIT_COMMIT_ID, GIT_TIMESTAMP_ID, GIT_BRANCH_ID and SPHINX_TAG, if any)
message ( STATUS "Branch is ${GIT_BRANCH_ID}, ${GIT_TIMESTAMP_ID}, ${SPH_GIT_COMMIT_ID}" )

configure_file ( "${SOURCE_DIR}/src/sphinxversion.h.in" "${BINARY_DIR}/config/gen_sphinxversion.h" @ONLY )
configure_file ( "${SOURCE_DIR}/CPackOptions.cmake.in" "${BINARY_DIR}/config/CPackOptions.cmake" @ONLY )




