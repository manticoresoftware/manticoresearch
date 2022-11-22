cmake_minimum_required ( VERSION 3.3 )

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
endif ()




