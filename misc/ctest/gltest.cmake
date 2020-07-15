# Initialize global vars with values came from outside (from gitlab-ci)
# This is main test suite which runs all the tests.
set ( CI_PROJECT_DIR "$ENV{CI_PROJECT_DIR}" )
set ( CTEST_BUILD_NAME "$ENV{CI_COMMIT_REF_NAME}" )
set ( CTEST_BUILD_CONFIGURATION "$ENV{CTEST_BUILD_CONFIGURATION}" )
set ( CTEST_CMAKE_GENERATOR "$ENV{CTEST_CMAKE_GENERATOR}" )
set ( LIBS_BUNDLE "$ENV{LIBS_BUNDLE}" )
set ( OPENSSL_ROOT_DIR "$ENV{OPENSSL_ROOT_DIR}" )
set ( DISABLE_GTESTS "$ENV{DISABLE_GTESTS}" )
set ( CTEST_REGEX "$ENV{CTEST_REGEX}" )
set ( WIN_TEST_CI "$ENV{WIN_TEST_CI}" )
set ( SEARCHD_CLI_EXTRA "$ENV{SEARCHD_CLI_EXTRA}" )
set_property ( GLOBAL PROPERTY Label P$ENV{CI_PIPELINE_ID} J$ENV{CI_JOB_ID} )

# set defaults for CI Windows test
if ( WIN_TEST_CI )
	if ( NOT CTEST_BUILD_CONFIGURATION )
		set ( CTEST_BUILD_CONFIGURATION "Debug" )
	endif()
endif()

MESSAGE (STATUS "WINTEST: ${WIN_TEST_CI}, config ${CTEST_BUILD_CONFIGURATION}")

if (NOT CTEST_CMAKE_GENERATOR)
	set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
endif ()

# platform specific options
set ( CTEST_SITE "$ENV{CI_SERVER_NAME} ${CTEST_BUILD_CONFIGURATION}" )

# fallback to run without ctest
if ( NOT CTEST_SOURCE_DIRECTORY )
	set ( CTEST_SOURCE_DIRECTORY ".." )
endif ()

if ( DEFINED ENV{WITH_PGSQL} )
	set ( WITH_PGSQL "$ENV{WITH_PGSQL}" )
elseif ( WIN_TEST_CI )
	set ( WITH_PGSQL 0 )
else()
	set ( WITH_PGSQL 1 )
endif()
# common test options
set ( CONFIG_OPTIONS "WITH_ODBC=1;WITH_RE2=1;WITH_STEMMER=1;WITH_PGSQL=${WITH_PGSQL};WITH_EXPAT=1;USE_SSL=1" )
set ( CTEST_BINARY_DIRECTORY "build" )

if ( WIN_TEST_CI )
	LIST(APPEND CONFIG_OPTIONS "CMAKE_INSTALL_PREFIX=.") # fixme! check if it could be used not only for win...
else()
	LIST(APPEND CONFIG_OPTIONS "CMAKE_INSTALL_DATADIR=${CTEST_SOURCE_DIRECTORY}/build/installdir")
endif()

if ( CTEST_BUILD_CONFIGURATION STREQUAL Debug )
	# configure coverage
	set ( WITH_COVERAGE TRUE )
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

if ( WIN_TEST_CI )
	LIST ( APPEND CONFIG_OPTIONS "WIN_TEST_CI=${WIN_TEST_CI}" )
endif()

SET ( CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE )
#ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})

#######################################################################
set ( CTESTCONFIG "${CTEST_BINARY_DIRECTORY}/CTestConfig.cmake")
file ( WRITE "${CTESTCONFIG}" "set ( CTEST_PROJECT_NAME \"Manticoresearch\" )\n" )
file ( APPEND "${CTESTCONFIG}" "set ( CTEST_NIGHTLY_START_TIME \"01:00:00 UTC\" )\n" )
file ( APPEND "${CTESTCONFIG}" "set ( CTEST_DROP_SITE_CDASH TRUE )\n" )

# configure memcheck
SET ( WITH_MEMCHECK FALSE )
#find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
#set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE ${CTEST_SOURCE_DIRECTORY}/tests/valgrind.supp)

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
if ( WIN_TEST_CI )
	ctest_build ( TARGET install )

	# dirty hack for icu: executables located at CTEST_BINARY_DIRECTORY/src/Debug/*.exe
	# same folder used as install prefix, and everything installed there. So we could push icu data
	# into expected place using installed icu from known location
	file(MAKE_DIRECTORY "${CTEST_BINARY_DIRECTORY}/src/share/icu")
	file(COPY "${CTEST_BINARY_DIRECTORY}/share/icu/" DESTINATION "${CTEST_BINARY_DIRECTORY}/src/share/icu/" FILES_MATCHING PATTERN "*.dat")

	# the same hack for dlls. They with binaries placed in the bin folder by installation
	file(COPY "${CTEST_BINARY_DIRECTORY}/bin/" DESTINATION "${CTEST_BINARY_DIRECTORY}/src/Debug/" FILES_MATCHING PATTERN "*.dll")
else ( WIN_TEST_CI )
	ctest_build ( TARGET install FLAGS "-j5" )
endif ()

if ( CTEST_REGEX )
	ctest_test ( RETURN_VALUE retcode INCLUDE "${CTEST_REGEX}" )
else()
	ctest_test ( RETURN_VALUE retcode )
endif()
#ctest_test ( START 24 END 25 RETURN_VALUE retcode )
#ctest_test ( STRIDE 50 )
#ctest_test ( STRIDE 50 EXCLUDE_LABEL RT RETURN_VALUE retcode )

if ( WITH_COVERAGE AND CTEST_COVERAGE_COMMAND )
	ctest_coverage ()
endif ( WITH_COVERAGE AND CTEST_COVERAGE_COMMAND )

if ( WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND )
	ctest_memcheck ()
endif ( WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND )

#ctest_submit ()

if ( retcode )
	MESSAGE ( FATAL_ERROR "tests failed with ${retcode} code" )
endif ()
