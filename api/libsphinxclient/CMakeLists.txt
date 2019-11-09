cmake_minimum_required ( VERSION 3.12 )
CMAKE_POLICY ( SET CMP0048 NEW )

# set default build type to Release (if nothing else provided from outside yet)
IF ( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES )
	SET ( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE )
	set_property ( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
			"MinSizeRel" "RelWithDebInfo" )
ENDIF ()

PROJECT ( SPHINXCLIENT
		LANGUAGES C
		VERSION 0.0.1
		DESCRIPTION "Pure C searchd client API library"
		HOMEPAGE_URL "http://manticoresearch.com"  )

# our mega-lib
add_library ( sphinxclient STATIC sphinxclient.c )
if ( NOT WIN32 )
	target_compile_options ( sphinxclient PRIVATE -g3 -Wall -O0 )
endif ()

# testing stuff
if ( NOT DISABLE_TESTING AND NOT DISABLE_GTESTS )

	add_executable (testcli test.c)
	target_link_libraries ( testcli sphinxclient )

	configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/smoke_data.csv ${CMAKE_CURRENT_BINARY_DIR}/smoke_data.csv COPYONLY )
	configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/smoke_test.conf ${CMAKE_CURRENT_BINARY_DIR}/smoke_test.conf COPYONLY )

	set ( test "\"Prepare API test\"" )
	add_test ( NAME "${test}" COMMAND indexer -c smoke_test.conf --all
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
	SET_TESTS_PROPERTIES ( "${test}" PROPERTIES LABELS "API"
			FIXTURES_SETUP APIINDEX  )

	set ( test "\"Perform API smoke test\"" )
	add_test ( NAME "${test}" COMMAND ${CMAKE_COMMAND}
			-D BINDIR=${MANTICORE_BINARY_DIR}
			-D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/smoke.cmake
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
	SET_TESTS_PROPERTIES ( "${test}" PROPERTIES LABELS "API"
			FIXTURES_REQUIRED APIINDEX )

endif()