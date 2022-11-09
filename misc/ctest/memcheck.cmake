# Initialize global vars with values came from outside (from gitlab-ci)
# This is main test suite which runs all the tests.
set ( CI_PROJECT_DIR "$ENV{CI_PROJECT_DIR}" )
set ( CTEST_BUILD_NAME "$ENV{CI_COMMIT_REF_NAME}" )
set ( CTEST_CONFIGURATION_TYPE "$ENV{CTEST_CONFIGURATION_TYPE}" )
set ( CTEST_CMAKE_GENERATOR "$ENV{CTEST_CMAKE_GENERATOR}" )
set ( LIBS_BUNDLE "$ENV{LIBS_BUNDLE}" )
set ( CTEST_REGEX "$ENV{CTEST_REGEX}" )
set ( SEARCHD_CLI_EXTRA "$ENV{SEARCHD_CLI_EXTRA}" )
set ( XTRA_FLAGS "$ENV{XTRA_FLAGS}" )
set_property ( GLOBAL PROPERTY Label P$ENV{CI_PIPELINE_ID} J$ENV{CI_JOB_ID} )

if (NOT CTEST_CMAKE_GENERATOR)
	set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
endif ()

# platform specific options
set ( CTEST_SITE "$ENV{CI_SERVER_NAME} ${CTEST_CONFIGURATION_TYPE}" )

# fallback to run without ctest
if ( NOT CTEST_SOURCE_DIRECTORY )
	set ( CTEST_SOURCE_DIRECTORY ".." )
endif ()

# just reference line for manual configuration
# cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_ODBC=1 -DWITH_RE2=1 -DWITH_STEMMER=1 -DWITH_POSTGRESQL=0 -DWITH_EXPAT=1 -DWITH_SSL=1 -DTEST_SPECIAL_EXTERNAL=1 -DDISABLE_MEMROUTINES=1 -DCMAKE_C_COMPILER=/usr/bin/clang-12 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-12 ..
# common test options
set ( CONFIG_OPTIONS "WITH_ODBC=1;WITH_RE2=1;WITH_STEMMER=1;WITH_POSTGRESQL=0;WITH_EXPAT=1;WITH_SSL=1;TEST_SPECIAL_EXTERNAL=1;DISABLE_MEMROUTINES=1" )
set ( CTEST_BINARY_DIRECTORY "${CTEST_SOURCE_DIRECTORY}/build" )

if ( LIBS_BUNDLE )
	LIST ( APPEND CONFIG_OPTIONS "LIBS_BUNDLE=${LIBS_BUNDLE}" )
endif ()

if ( SEARCHD_CLI_EXTRA )
	LIST ( APPEND CONFIG_OPTIONS "SEARCHD_CLI_EXTRA=${SEARCHD_CLI_EXTRA}" )
endif()

SET ( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE )
#ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})

#######################################################################
if ($ENV{CDASH_UPLOAD})
configure_file ("$ENV{CDASH_UPLOAD}" "${CTEST_BINARY_DIRECTORY}/CTestConfig.cmake" COPYONLY)
endif()

# configure memcheck
SET ( WITH_MEMCHECK TRUE )
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
get_filename_component (CTEST_MEMORYCHECK_SUPPRESSIONS_FILE "${CTEST_SOURCE_DIRECTORY}/test/valgrind.supp" ABSOLUTE)

# configure update (will log git rev id)
find_program ( CTEST_GIT_COMMAND NAMES git )
SET ( CTEST_UPDATE_COMMAND "${CTEST_GIT_COMMAND}" )
SET ( CTEST_UPDATE_VERSION_ONLY ON )

set ( CMAKE_CALL "${CMAKE_COMMAND} \"-G${CTEST_CMAKE_GENERATOR}\" -DCMAKE_BUILD_TYPE:STRING=${CTEST_CONFIGURATION_TYPE}" )
foreach ( OPTION ${CONFIG_OPTIONS} )
	set ( CMAKE_CALL "${CMAKE_CALL} -D${OPTION}" )
endforeach ()
foreach (OPTION ${XTRA_FLAGS})
	set ( CMAKE_CALL "${CMAKE_CALL} -D${OPTION}" )
endforeach ()
set ( CTEST_CONFIGURE_COMMAND "${CMAKE_CALL} \"${CTEST_SOURCE_DIRECTORY}\"" )

# will not write and count warnings in auto-generated files of lexer
set ( CTEST_CUSTOM_WARNING_EXCEPTION ".*flexsphinx.*" )

# Do the test suite
ctest_start ( "Continuous" )
ctest_update ()
ctest_configure ()
ctest_build ()

if ( CTEST_REGEX )
#	ctest_test ( RETURN_VALUE retcode INCLUDE "${CTEST_REGEX}" REPEAT UNTIL_PASS:${RETRIES})
else()
#	ctest_test ( RETURN_VALUE retcode REPEAT UNTIL_PASS:${RETRIES})
endif()

if ( WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND )
	set (ENV{vlg} "${CTEST_MEMORYCHECK_COMMAND}")
	get_filename_component (CTEST_MEMORYCHECK_COMMAND "${CTEST_SOURCE_DIRECTORY}/valgrind" ABSOLUTE)
	if ($ENV{MEMORYCHECK_COMMAND_OPTIONS})
		set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "$ENV{MEMORYCHECK_COMMAND_OPTIONS}")
	endif ()
#	set (CTEST_MEMORYCHECK_COMMAND_OPTIONS "-q --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=50 --track-origins=yes")
#	set (CTEST_MEMORYCHECK_COMMAND_OPTIONS "-q --tool=memcheck --leak-check=full --show-reachable=no --num-callers=50 --track-origins=yes")
	if (CTEST_REGEX)
		ctest_memcheck (INCLUDE_LABEL UBER INCLUDE "${CTEST_REGEX}" RETURN_VALUE retcode)
	else ()
		ctest_memcheck (INCLUDE_LABEL UBER RETURN_VALUE retcode)
	endif ()
endif ( WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND )

ctest_submit ()

if ( retcode )
	MESSAGE ( STATUS "tests failed with ${retcode} code, but we don't fail as it is memcheck pass" )
endif ()
