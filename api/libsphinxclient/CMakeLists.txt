cmake_minimum_required ( VERSION 3.12 )

FOREACH ( policy CMP0048 CMP0110 )
	IF (POLICY ${policy})
		CMAKE_POLICY(SET ${policy} NEW)
	ENDIF ()
ENDFOREACH ()

# Set a default build type for single-configuration CMake generators if no build type is set.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif ()

PROJECT ( SPHINXCLIENT
		LANGUAGES C
		VERSION 0.0.1
		DESCRIPTION "Pure C searchd client API library"
		HOMEPAGE_URL "https://manticoresearch.com"  )

# our mega-lib
add_library ( sphinxclient STATIC sphinxclient.c )
target_compile_options (sphinxclient PRIVATE $<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang,GNU>:-g3 -Wall -O0>)

target_compile_options (sphinxclient PRIVATE $<$<COMPILE_LANG_AND_ID:C,Clang>:
	-Wno-reserved-id-macro -Wno-extra-semi-stmt -Wno-cast-qual -Wno-sign-conversion -Wno-language-extension-token
	-Wno-nonportable-system-include-path -Wno-format-nonliteral -Wno-missing-prototypes -Wno-sign-compare
	-Wno-deprecated-declarations -Wno-cast-align -Wno-newline-eof>)

if (NOT BUILD_TESTING OR DISABLE_GTESTS OR WIN32 OR TEST_SPECIAL_EXTERNAL )
	return()
endif()

# testing stuff
add_executable (testcli test.c)
target_link_libraries ( testcli sphinxclient )

configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/smoke_data.csv ${CMAKE_CURRENT_BINARY_DIR}/smoke_data.csv COPYONLY )
configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/smoke_test.conf ${CMAKE_CURRENT_BINARY_DIR}/smoke_test.conf COPYONLY )

# since cmake 3.18 names need fixup
include(fixup_test_name)

fixup_test_name ( test "Prepare API test" )
add_test ( NAME ${test} COMMAND indexer -c smoke_test.conf --all
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
SET_TESTS_PROPERTIES ( ${test} PROPERTIES LABELS API FIXTURES_SETUP APIINDEX  )

fixup_test_name ( test "Perform API smoke test" )
add_test ( NAME ${test} COMMAND ${CMAKE_COMMAND}
		-D SEARCHD=$<TARGET_FILE:searchd>
		-D CLI=$<TARGET_FILE:testcli>
		-D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/smoke.cmake
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
SET_TESTS_PROPERTIES ( ${test} PROPERTIES LABELS API FIXTURES_REQUIRED APIINDEX )