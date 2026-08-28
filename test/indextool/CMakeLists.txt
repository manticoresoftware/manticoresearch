cmake_minimum_required ( VERSION 3.17 )

set(PORT 19306)

if (WIN32 OR CYGWIN)
	message (STATUS "Skip add indextool test on windows")
else ()
	configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/indextool.conf ${MANTICORE_BINARY_DIR}/test/indextool.conf @ONLY )
	add_test ( NAME Perform_indextool COMMAND ${CMAKE_COMMAND}
			-D SEARCHD=$<TARGET_FILE:searchd>
			-D INDEXTOOL=$<TARGET_FILE:indextool>
			-D PORT=${PORT}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/test.cmake
			WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}/test" )
	SET_TESTS_PROPERTIES ( Perform_indextool PROPERTIES LABELS INDEXTOOL )
    if ( COMMAND special_ubertest_properties )
		special_ubertest_properties ( Perform_indextool )
	endif ()
	add_test ( NAME Apply_killlists_with_legacy_json_header COMMAND ${CMAKE_COMMAND}
			-D INDEXER=$<TARGET_FILE:indexer>
			-D INDEXTOOL=$<TARGET_FILE:indextool>
			-D LEGACY_FIXTURE=${CMAKE_SOURCE_DIR}/test/test_406/data
			-P ${CMAKE_CURRENT_SOURCE_DIR}/test_apply_killlists_legacy_json.cmake
			WORKING_DIRECTORY "${MANTICORE_BINARY_DIR}/test" )
	SET_TESTS_PROPERTIES ( Apply_killlists_with_legacy_json_header PROPERTIES LABELS INDEXTOOL )
endif ()




