# this cmake file fully replaces all old things like svnxrev, hgxrev, etc scripts.
cmake_minimum_required (VERSION 2.8)
if(CMAKE_VERSION VERSION_GREATER "3.1")
	CMAKE_POLICY(SET CMP0053 OLD)
endif()

# first try to use binary git
if (EXISTS "${SOURCE_DIR}/.git")
	find_package(Git)
	if (GIT_FOUND)
		execute_process(COMMAND "${GIT_EXECUTABLE}" log -1 --format=%h
			WORKING_DIRECTORY "${SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE HEAD_HASH
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		execute_process(COMMAND "${GIT_EXECUTABLE}" status -s -b
			WORKING_DIRECTORY "${SOURCE_DIR}"
			RESULT_VARIABLE res
			OUTPUT_VARIABLE HEAD_BRANCH
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		string(REGEX REPLACE "\n.*$" "" HEAD_BRANCH "${HEAD_BRANCH}")
		string(REPLACE "## " "" HEAD_BRANCH "${HEAD_BRANCH}")
	else (GIT_FOUND)
		# first try to use git revision as the most modern flow
		set(GIT_DIR "${SOURCE_DIR}/.git")
		if(EXISTS "${GIT_DIR}/HEAD")

			message (STATUS "Using git working copy to provide branch and tag data")
			set(GIT_DATA "${BINARY_DIR}/CMakeFiles/git-data")
			if(NOT EXISTS "${GIT_DATA}")
				file(MAKE_DIRECTORY "${GIT_DATA}")
			endif()

			set(HEAD_FILE "${GIT_DIR}/HEAD")

			file(READ ${HEAD_FILE} HEAD_CONTENTS LIMIT 1024)
			string(STRIP "${HEAD_CONTENTS}" HEAD_CONTENTS)
			if(HEAD_CONTENTS MATCHES "ref")
				# named branch
				string(REPLACE "ref: " "" HEAD_REF "${HEAD_CONTENTS}")
				if(EXISTS "${GIT_DIR}/${HEAD_REF}")
					configure_file("${GIT_DIR}/${HEAD_REF}" "${GIT_DATA}/head-ref" COPYONLY)
				else()
					configure_file("${GIT_DIR}/packed-refs" "${GIT_DATA}/packed-refs" COPYONLY)
					file(READ "${GIT_DATA}/packed-refs" PACKED_REFS)
					if(${PACKED_REFS} MATCHES "([0-9a-z]*) ${HEAD_REF}")
						set(HEAD_HASH "${CMAKE_MATCH_1}")
					endif()
				endif()
				get_filename_component(HEAD_BRANCH ${GIT_DIR}/${HEAD_REF} NAME)
				string(STRIP "${HEAD_BRANCH}" HEAD_BRANCH)
			else()
				# detached HEAD
				configure_file("${GIT_DIR}/HEAD" "${GIT_DATA}/head-ref" COPYONLY)
				set(HEAD_BRANCH "")
			endif()

			if(NOT HEAD_HASH)
				file(READ "${GIT_DATA}/head-ref" HEAD_HASH LIMIT 1024)
				string(STRIP "${HEAD_HASH}" HEAD_HASH)
			endif()

			if (HEAD_HASH)
				string(SUBSTRING "${HEAD_HASH}" 0 7 HEAD_HASH)
			endif (HEAD_HASH)
		endif (EXISTS "${GIT_DIR}/HEAD")
	endif(GIT_FOUND)
endif()

# git pass gave nothing. Try, if it is an svn
if (NOT HEAD_HASH)
	if (EXISTS "${SOURCE_DIR}/.svn")
		find_package(Subversion)
		if (SUBVERSION_FOUND)
			message (STATUS "Using svn working copy to provide branch and tag data")
			Subversion_WC_INFO(${SOURCE_DIR} MY)
			set (HEAD_HASH "${MY_WC_REVISION}")
			get_filename_component(HEAD_BRANCH ${MY_WC_URL} NAME)
		endif (SUBVERSION_FOUND)
	endif()
endif (NOT HEAD_HASH)

# still nothing. Use the last chance - extract version from ID in the sources/headers
if (NOT HEAD_HASH)
	message (STATUS "Using full source scan to provide branch and tag data")
	file(GLOB RAWSRC "${SOURCE_DIR}/src/*.cpp" "${SOURCE_DIR}/src/*.h")
	set (ID 0)
	foreach(RAW ${RAWSRC})
		file(READ "${RAW}" CNT LIMIT 1024)
		if ( CNT MATCHES "\\\$Id: [a-zA-Z.]+ ([0-9]+)" )
			set(IDD ${CMAKE_MATCH_1})
			message(STATUS "Matched ${RAW} ${IDD}")
			if (IDD GREATER ID)
				set (ID ${IDD})
			endif()
		endif()
	endforeach()
	set (HEAD_HASH "${ID}")
endif (NOT HEAD_HASH)

message(STATUS "Branch is ${HEAD_BRANCH}, hash is ${HEAD_HASH}")
if (HEAD_BRANCH)
	set (HEAD_TAGREV "${HEAD_BRANCH}_${HEAD_HASH}")
else (HEAD_BRANCH)
	set (HEAD_TAGREV "${HEAD_HASH}")
endif (HEAD_BRANCH)

if ( SPHINX_TAG )
	set (HEAD_BRANCH "${HEAD_BRANCH}, tag ${SPHINX_TAG}")
	set (HEAD_TAGREV "${HEAD_TAGREV}, tag ${SPHINX_TAG}")
endif ( SPHINX_TAG )

file(WRITE ${BINARY_DIR}/config/sphinxversion.h.txt
	"#define SPH_SVN_TAG \"@HEAD_BRANCH@\"\n
	#define SPH_SVN_REV @HEAD_HASH@\n
	#define SPH_SVN_REVSTR \"@HEAD_HASH@\"\n
	#define SPH_GIT_COMMIT_ID \"@HEAD_HASH@\"\n
	#define SPH_SVN_TAGREV \"@HEAD_TAGREV@\"\n")

configure_file("${BINARY_DIR}/config/sphinxversion.h.txt" "${BINARY_DIR}/config/gen_sphinxversion.h" @ONLY)

