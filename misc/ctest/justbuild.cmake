# Initialize global vars with values came from outside (from gitlab-ci)
# This is main test suite which runs all the tests.
set ( CI_PROJECT_DIR "$ENV{CI_PROJECT_DIR}" )
set ( CTEST_BUILD_NAME "$ENV{CI_COMMIT_REF_NAME}" )
set ( CTEST_CONFIGURATION_TYPE "$ENV{CTEST_CONFIGURATION_TYPE}" )
set ( CTEST_CMAKE_GENERATOR "$ENV{CTEST_CMAKE_GENERATOR}" )
set ( LIBS_BUNDLE "$ENV{LIBS_BUNDLE}" )
set ( UNITY_BUILD "$ENV{UNITY_BUILD}" )
set_property ( GLOBAL PROPERTY Label P$ENV{CI_PIPELINE_ID} J$ENV{CI_JOB_ID} )

# platform specific options
set ( CTEST_SITE "$ENV{CI_SERVER_NAME} build ${CTEST_CONFIGURATION_TYPE}" )

if ( NOT WITH_GALERA )
	set (WITH_GALERA 0 )
endif()

# common test options
set ( CONFIG_OPTIONS "WITH_ODBC=1;WITH_RE2=1;WITH_STEMMER=1;WITH_POSTGRESQL=1;WITH_EXPAT=1;BUILD_TESTING=0" )
LIST ( APPEND CONFIG_OPTIONS "LIBS_BUNDLE=${LIBS_BUNDLE}" )
LIST ( APPEND CONFIG_OPTIONS "WITH_GALERA=${WITH_GALERA}" )

if ( NOT CTEST_SOURCE_DIRECTORY )
	set ( CTEST_SOURCE_DIRECTORY "${CI_PROJECT_DIR}")
endif()

# fallback to run without ctest
if ( NOT CTEST_SOURCE_DIRECTORY )
	set ( CTEST_SOURCE_DIRECTORY "." )
endif ()

if ( NOT CTEST_CMAKE_GENERATOR )
	set ( CTEST_CMAKE_GENERATOR "Unix Makefiles" )
endif ()

if (UNITY_BUILD)
	LIST ( APPEND CONFIG_OPTIONS "CMAKE_UNITY_BUILD=${UNITY_BUILD}" )
endif ()

set ( CTEST_BINARY_DIRECTORY "${CTEST_SOURCE_DIRECTORY}/build" )
SET ( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE )
#ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})

#######################################################################
set ( CTESTCONFIG "${CTEST_BINARY_DIRECTORY}/CTestConfig.cmake" )
file ( WRITE "${CTESTCONFIG}" "set ( CTEST_PROJECT_NAME \"Manticoresearch\" )\n" )
file ( APPEND "${CTESTCONFIG}" "set ( CTEST_NIGHTLY_START_TIME \"01:00:00 UTC\" )\n" )
file ( APPEND "${CTESTCONFIG}" "set ( CTEST_DROP_SITE_CDASH TRUE )\n" )

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
include ( ProcessorCount )
ProcessorCount ( N )
if (NOT N EQUAL 0)
	if (NOT CTEST_CMAKE_GENERATOR STREQUAL "Visual Studio 16 2019")
		set ( CTEST_BUILD_FLAGS -j${N} )
	endif ()
	set ( ctest_test_args ${ctest_test_args} PARALLEL_LEVEL ${N} )
endif ()
ctest_build ( RETURN_VALUE retcode )

if ( retcode )
	MESSAGE ( FATAL_ERROR "tests failed with ${retcode} code" )
endif ()