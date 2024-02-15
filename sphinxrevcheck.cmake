cmake_minimum_required ( VERSION 3.17 )

# this file included by cpack config in order to detect if configured and build version are the same
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

	# extract short has as CHECK_GIT_COMMIT_ID
	execute_process ( COMMAND "${GIT_EXECUTABLE}" log -1 --format=%h
			WORKING_DIRECTORY "${MANTICORE_SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE CHECK_GIT_COMMIT_ID
			OUTPUT_STRIP_TRAILING_WHITESPACE )
	set ( CHECK_GIT_COMMIT_ID "${CHECK_GIT_COMMIT_ID}" PARENT_SCOPE )

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
		set ( CHECK_GIT_COMMIT_ID "${GIT_COMMIT_ID}" PARENT_SCOPE )
	endif ()
endfunction ()

# function definitions finished, execution starts from here
##################################

# first try to use binary git
guess_from_git ()

# 2-nd try - if we build from git archive. Correct hash and date provided then, but no branch
if (NOT CHECK_GIT_COMMIT_ID)
  message ( STATUS "guess_from_git failed. 2nd try - extract_from_git_slug" )
	extract_from_git_slug ( "${MANTICORE_SOURCE_DIR}/src/sphinxversion.h.in" )
endif ()

if (NOT CHECK_GIT_COMMIT_ID)
	message ( STATUS "extract_from_git_slug failed. 3rd try - forcing setting CHECK_GIT_COMMIT_ID" )
	set ( CHECK_GIT_COMMIT_ID "DEADBEEF" )
endif ()

message ( STATUS "CHECK_GIT_COMMIT_ID=${CHECK_GIT_COMMIT_ID}" )

if (NOT CHECK_GIT_COMMIT_ID STREQUAL "${CONFIGURED_GIT_COMMIT_ID}")
	message ( FATAL_ERROR "Current commit ${CHECK_GIT_COMMIT_ID} differs from the stored ${CONFIGURED_GIT_COMMIT_ID}. Run 'cmake .' in build dir to fix" )
endif ()
