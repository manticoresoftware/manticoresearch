cmake_minimum_required ( VERSION 3.11.0 )

# Read docs at https://github.com/google/googletest.git
include ( GetGoogletest )
if (NOT TARGET GTest::gmock_main)
	return ()
endif ()

set ( GTESTS_SRC
		gtests_rtstuff.cpp
		gtests_tokenizer.cpp
		gtests_text.cpp
		gtests_functions.cpp
		gtests_globalstate.cpp
		gtests_searchd.cpp
		gtests_filter.cpp
		gtests_searchdaemon.cpp
		gtests_stringbuilder.cpp
		gtests_strfmt.cpp
		gtests_pqstuff.cpp
		gtests_json.cpp
		gtests_threadstuff.cpp )

add_executable ( gmanticoretest ${GTESTS_SRC} )
target_include_directories ( gmanticoretest PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( gmanticoretest
		GTest::gmock_main
		lmanticore
		lsearchd
		searchd_ssl
		source_svpipe )

if (WITH_EXPAT)
	target_link_libraries ( gmanticoretest source_xmlpipe2 )
endif ()

target_compile_options ( gmanticoretest PRIVATE $<$<OR:$<COMPILE_LANG_AND_ID:CXX,GNU>,$<COMPILE_LANG_AND_ID:C,GNU>>:-Wno-array-bounds -Wno-stringop-truncation -Wno-restrict -Wno-stringop-overflow> )

set ( CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST )

include ( GoogleTest )
GTEST_DISCOVER_TESTS ( gmanticoretest PROPERTIES LABELS GOOGLE )
message ( STATUS "Discovered and added individual google-tests" )

if (WIN32)
	getruntime ( libs common )
	getruntime ( libs indexer )
	getruntime ( libs daemon )
	add_custom_command ( TARGET gmanticoretest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${libs} "$<TARGET_FILE_DIR:gmanticoretest>" VERBATIM )
endif ()