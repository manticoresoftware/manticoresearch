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
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
	target_compile_options ( sphinxclient PRIVATE /clang:-Wall )
	target_compile_definitions ( sphinxclient PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS=1 )
else ()
	target_compile_options ( sphinxclient PRIVATE -g3 -Wall -O0 )
endif ()
target_link_libraries ( sphinxclient PRIVATE $<$<PLATFORM_ID:Windows>:wsock32> )

if (NOT BUILD_TESTING OR DISABLE_GTESTS OR WIN32 OR TEST_SPECIAL_EXTERNAL )
	return()
endif()

# testing stuff
add_executable (testcli test.c)
target_link_libraries ( testcli sphinxclient )

add_executable (sphinxclient_serialization_test serialization_test.c)

find_program ( PHP_EXECUTABLE php )
find_program ( RUBY_EXECUTABLE ruby )

configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/smoke_data.csv ${CMAKE_CURRENT_BINARY_DIR}/smoke_data.csv COPYONLY )
configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/smoke_test.conf ${CMAKE_CURRENT_BINARY_DIR}/smoke_test.conf COPYONLY )

# since cmake 3.18 names need fixup
include(fixup_test_name)

fixup_test_name ( test "Perform API smoke test" )
add_test ( NAME ${test} COMMAND ${CMAKE_COMMAND}
		-D SEARCHD=$<TARGET_FILE:searchd>
		-D INDEXER=$<TARGET_FILE:indexer>
		-D CLI=$<TARGET_FILE:testcli>
		-D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/smoke.cmake
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
SET_TESTS_PROPERTIES ( ${test} PROPERTIES LABELS API )

fixup_test_name ( serialization_test "C API explicit ranker serialization" )
add_test ( NAME ${serialization_test} COMMAND sphinxclient_serialization_test )
SET_TESTS_PROPERTIES ( ${serialization_test} PROPERTIES LABELS API )

if ( PHP_EXECUTABLE )
	fixup_test_name ( php_serialization_test "PHP API explicit ranker serialization" )
	add_test ( NAME ${php_serialization_test} COMMAND ${PHP_EXECUTABLE} ${CMAKE_SOURCE_DIR}/api/sphinxapi_ranker_test.php )
	SET_TESTS_PROPERTIES ( ${php_serialization_test} PROPERTIES LABELS API )
endif ()

if ( RUBY_EXECUTABLE )
	fixup_test_name ( ruby_serialization_test "Ruby API explicit ranker serialization" )
	add_test ( NAME ${ruby_serialization_test} COMMAND ${RUBY_EXECUTABLE} ${CMAKE_SOURCE_DIR}/api/ruby/serialization_test.rb )
	SET_TESTS_PROPERTIES ( ${ruby_serialization_test} PROPERTIES LABELS API )
endif ()
