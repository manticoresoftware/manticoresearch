# this cmake file fully replaces all old things like svnxrev, hgxrev, etc scripts.
cmake_minimum_required ( VERSION 2.8 )
if ( CMAKE_VERSION VERSION_GREATER "3.1" )
	CMAKE_POLICY ( SET CMP0053 OLD )
endif ()

# first try to use binary git
if ( EXISTS "${SOURCE_DIR}/.git" )
	find_package ( Git QUIET )
	if ( GIT_FOUND )
		execute_process ( COMMAND "${GIT_EXECUTABLE}" log -1 --format=%h
				WORKING_DIRECTORY "${SOURCE_DIR}"
				RESULT_VARIABLE res
				OUTPUT_VARIABLE SPH_GIT_COMMIT_ID
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE )
			# it would be --date=format:%y%m%d, but old git on centos doesn't understand it
		execute_process ( COMMAND "${GIT_EXECUTABLE}" log -1 --date=short --format=%ad
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
		set ( GIT_BRANCH_ID "git branch ${GIT_BRANCH_ID}" )
		string ( REPLACE "-" "" GIT_TIMESTAMP_ID "${GIT_TIMESTAMP_ID}" )
		string ( SUBSTRING "${GIT_TIMESTAMP_ID}" 2 -1 GIT_TIMESTAMP_ID )
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
		set (GIT_BRANCH_ID "from tarball")
	else ()
		# final flow: check source dir name for matching tarball
		get_filename_component ( DIR ${SOURCE_DIR} NAME )
		if ( NOT "${DIR}" MATCHES "^manticore-([0-9]+\\.[0-9]+\\.[0-9]+)-([0-9][0-9][0-9][0-9][0-9][0-9])-([0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]+)-?(.*)?$" )
			message ( FATAL_ERROR "Git not found, or the sources are not git clone, or not located in the folder originally unpacked from tarball, or not contain pre-created sphinxversion.h header. Please, put this file to your src/ folder manually." )
		endif()
		set ( VERNUMBERS "${CMAKE_MATCH_1}" )
		set ( GIT_TIMESTAMP_ID "${CMAKE_MATCH_2}" )
		set ( SPH_GIT_COMMIT_ID "${CMAKE_MATCH_3}")
		set ( SPHINX_TAG "${CMAKE_MATCH_4}" )
		set ( GIT_BRANCH_ID "from the folder '${DIR}'" )
	endif ()
endif ()

# extract version number string from sphinx.h
if ( NOT VERNUMBERS )
	FILE ( STRINGS "${SOURCE_DIR}/src/sphinxversion.h.in" _STRINGS LIMIT_COUNT 500
			REGEX "^#define[ \t]+VERNUMBERS.*" )
	STRING ( REGEX REPLACE ".*\"(.*)\"(.*)$" "\\1" VERNUMBERS "${_STRINGS}" )
endif()

# All info collected (we need SPH_GIT_COMMIT_ID, GIT_TIMESTAMP_ID, GIT_BRANCH_ID and SPHINX_TAG, if any)
message ( STATUS "Version ${VERNUMBERS} ${SPH_GIT_COMMIT_ID}@${GIT_TIMESTAMP_ID}, ${GIT_BRANCH_ID}" )

configure_file ( "${SOURCE_DIR}/src/sphinxversion.h.in" "${BINARY_DIR}/config/gen_sphinxversion.h" )
configure_file ( "${SOURCE_DIR}/dist/CPackOptions.cmake.in" "${BINARY_DIR}/config/CPackOptions.cmake" @ONLY )


