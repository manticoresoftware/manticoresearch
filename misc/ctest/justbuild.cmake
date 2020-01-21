# Initialize global vars with values came from outside (from gitlab-ci)
# This is main test suite which runs all the tests.
set ( CI_PROJECT_DIR "$ENV{CI_PROJECT_DIR}" )
set ( CTEST_BUILD_NAME "$ENV{CI_COMMIT_REF_NAME}" )
set ( CTEST_BUILD_CONFIGURATION "$ENV{CTEST_BUILD_CONFIGURATION}" )
set ( CTEST_CMAKE_GENERATOR "$ENV{CTEST_CMAKE_GENERATOR}" )
set ( LIBS_BUNDLE "$ENV{LIBS_BUNDLE}" )
set ( USE_GALERA "$ENV{USE_GALERA}")
set ( WITH_ICU "$ENV{WITH_ICU}")
set ( OPENSSL_ROOT_DIR "$ENV{OPENSSL_ROOT_DIR}" )
set_property ( GLOBAL PROPERTY Label P$ENV{CI_PIPELINE_ID} J$ENV{CI_JOB_ID} )

# platform specific options
set ( CTEST_SITE "$ENV{CI_SERVER_NAME} build ${CTEST_BUILD_CONFIGURATION}" )

if ( NOT USE_GALERA )
	set ( USE_GALERA 0 )
endif()

if ( NOT WITH_ICU )
	set ( WITH_ICU 0 )
endif()

# common test options
set ( CONFIG_OPTIONS "WITH_ODBC=1;WITH_RE2=1;WITH_STEMMER=1;WITH_PGSQL=1;WITH_EXPAT=1;DISABLE_TESTING=1" )
LIST ( APPEND CONFIG_OPTIONS "LIBS_BUNDLE=${LIBS_BUNDLE}" )
LIST ( APPEND CONFIG_OPTIONS "USE_GALERA=${USE_GALERA}" )
LIST ( APPEND CONFIG_OPTIONS "WITH_ICU=${WITH_ICU}" )

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

set ( CMAKE_CALL "${CMAKE_COMMAND} \"-G${CTEST_CMAKE_GENERATOR}\" -DCMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}" )
foreach ( OPTION ${CONFIG_OPTIONS} )
	set ( CMAKE_CALL "${CMAKE_CALL} -D${OPTION}" )
endforeach ()
set ( CTEST_CONFIGURE_COMMAND "${CMAKE_CALL} \"${CTEST_SOURCE_DIRECTORY}\"" )

# will not write and count warnings in auto-generated files of lexer
set ( CTEST_CUSTOM_WARNING_EXCEPTION ".*flexsphinx.*" )

# Do the test suite
ctest_start ( "Continuous" )
ctest_update ()
ctest_configure ()
ctest_build ( RETURN_VALUE retcode )

if ( retcode )
	MESSAGE ( FATAL_ERROR "tests failed with ${retcode} code" )
endif ()