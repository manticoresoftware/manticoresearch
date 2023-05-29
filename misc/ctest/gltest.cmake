# Initialize global vars with values came from outside (from gitlab-ci)
# This is main test suite which runs all the tests.
set ( CI_PROJECT_DIR "$ENV{CI_PROJECT_DIR}" )
set ( CTEST_BUILD_NAME "$ENV{CI_COMMIT_REF_NAME}" )
set ( CTEST_CONFIGURATION_TYPE "$ENV{CTEST_CONFIGURATION_TYPE}" )
set ( CTEST_CMAKE_GENERATOR "$ENV{CTEST_CMAKE_GENERATOR}" )
set ( LIBS_BUNDLE "$ENV{LIBS_BUNDLE}" )
set ( DISABLE_GTESTS "$ENV{DISABLE_GTESTS}" )
set ( CTEST_REGEX "$ENV{CTEST_REGEX}" )
set ( CTEST_START "$ENV{CTEST_START}" )
set ( CTEST_END "$ENV{CTEST_END}" )
set ( SEARCHD_CLI_EXTRA "$ENV{SEARCHD_CLI_EXTRA}" )
set ( WITH_COVERAGE "$ENV{WITH_COVERAGE}" )
set ( NO_TESTS "$ENV{NO_TESTS}" )
set ( NO_BUILD "$ENV{NO_BUILD}" )
set_property ( GLOBAL PROPERTY Label P$ENV{CI_PIPELINE_ID} J$ENV{CI_JOB_ID} )

# how may times try the test before it is considered failed
set (RETRIES 5)

if (NOT CTEST_CMAKE_GENERATOR)
	set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
endif ()

# platform specific options
set ( CTEST_SITE "$ENV{CI_SERVER_NAME} ${CTEST_CONFIGURATION_TYPE}" )

# fallback to run without ctest
if ( NOT CTEST_SOURCE_DIRECTORY )
	set ( CTEST_SOURCE_DIRECTORY ".." )
endif ()

if (WITH_COVERAGE)
	set (WITH_POSTGRESQL "$ENV{WITH_POSTGRESQL}" )
endif()
# common test options
set ( CONFIG_OPTIONS "WITH_ODBC=1;WITH_RE2=1;WITH_STEMMER=1;WITH_POSTGRESQL=${WITH_POSTGRESQL};WITH_EXPAT=1;WITH_SSL=1" )
set ( CTEST_BINARY_DIRECTORY "build" )

if (WITH_COVERAGE)
	# configure coverage
	find_program ( CTEST_COVERAGE_COMMAND NAMES gcov )
	LIST ( APPEND CONFIG_OPTIONS "COVERAGE_TEST=1" )
	LIST ( APPEND CTEST_CUSTOM_COVERAGE_EXCLUDE "googletest-src/.*" )
endif ()

if ( DISABLE_GTESTS )
	LIST ( APPEND CONFIG_OPTIONS "DISABLE_GTESTS=${DISABLE_GTESTS}" )
endif ()

if ( LIBS_BUNDLE )
	LIST ( APPEND CONFIG_OPTIONS "LIBS_BUNDLE=${LIBS_BUNDLE}" )
endif ()

if ( SEARCHD_CLI_EXTRA )
	LIST ( APPEND CONFIG_OPTIONS "SEARCHD_CLI_EXTRA=${SEARCHD_CLI_EXTRA}" )
endif()

SET ( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE )
#ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})

#######################################################################
file ( WRITE "${CTEST_BINARY_DIRECTORY}/CTestConfig.cmake" "
set ( CTEST_PROJECT_NAME \"Manticoresearch\" )
set ( CTEST_NIGHTLY_START_TIME \"01:00:00 UTC\" )
set ( CTEST_DROP_SITE_CDASH TRUE )
" )

# configure memcheck
SET ( WITH_MEMCHECK FALSE )
#find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
#set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE ${CTEST_SOURCE_DIRECTORY}/tests/valgrind.supp)

# configure update (will log git rev id)
find_program ( CTEST_GIT_COMMAND NAMES git )
SET ( CTEST_UPDATE_COMMAND "${CTEST_GIT_COMMAND}" )
SET ( CTEST_UPDATE_VERSION_ONLY ON )

set ( CMAKE_CALL "${CMAKE_COMMAND} -G \"${CTEST_CMAKE_GENERATOR}\" -DCMAKE_BUILD_TYPE:STRING=${CTEST_CONFIGURATION_TYPE}" )
foreach ( OPTION ${CONFIG_OPTIONS} )
	set ( CMAKE_CALL "${CMAKE_CALL} -D${OPTION}" )
endforeach ()
set ( CTEST_CONFIGURE_COMMAND "${CMAKE_CALL} ${CTEST_SOURCE_DIRECTORY}" )

# will not write and count warnings in auto-generated files of lexer
set ( CTEST_CUSTOM_WARNING_EXCEPTION ".*flexsphinx.*" )

# Do the test suite
ctest_start ( "Continuous" )
ctest_update ()
ctest_configure ()

if (NOT NO_BUILD)
	include ( ProcessorCount )
	ProcessorCount ( N )
	if (NOT N EQUAL 0)
		if ( NOT CTEST_CMAKE_GENERATOR STREQUAL "Visual Studio 16 2019" )
			set ( CTEST_BUILD_FLAGS -j${N} )
		endif()
		set ( ctest_test_args ${ctest_test_args} PARALLEL_LEVEL ${N} )
	endif ()

	ctest_build ( ${ctest_test_args} )
endif()

if (NO_TESTS)
	return()
endif()

if ( CTEST_REGEX )
	ctest_test ( RETURN_VALUE retcode INCLUDE "${CTEST_REGEX}" REPEAT UNTIL_PASS:${RETRIES})
else()
  if ( CTEST_START AND CTEST_END )
  	ctest_test ( START ${CTEST_START} END ${CTEST_END} RETURN_VALUE retcode REPEAT UNTIL_PASS:${RETRIES})
	else()
		ctest_test ( RETURN_VALUE retcode REPEAT UNTIL_PASS:${RETRIES})
	endif()
endif()

#ctest_test ( STRIDE 50 )
#ctest_test ( STRIDE 50 EXCLUDE_LABEL RT RETURN_VALUE retcode )

if ( WITH_COVERAGE AND CTEST_COVERAGE_COMMAND )
	ctest_coverage ( CAPTURE_CMAKE_ERROR foo )
	if (foo)
		MESSAGE ( STATUS "coverage failed, but we continue..." )
	endif ()
endif ( WITH_COVERAGE AND CTEST_COVERAGE_COMMAND )

if ( WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND )
	ctest_memcheck ()
endif ( WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND )

#ctest_submit ()

if ( retcode )
	MESSAGE ( FATAL_ERROR "tests failed with ${retcode} code" )
endif ()
