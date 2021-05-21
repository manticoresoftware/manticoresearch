cmake_minimum_required ( VERSION 3.11.0 )

# Read docs at https://github.com/google/googletest.git
include ( GetGoogletest )
if (TARGET GTest::gmock_main)

	set ( GTESTS_SRC
			gtests_rtstuff.cpp
			gtests_tokenizer.cpp
			gtests_text.cpp
			gtests_functions.cpp
			gtests_globalstate.cpp
			gtests_searchd.cpp
			gtests_filter.cpp
			gtests_searchdaemon.cpp
			gtests_pqstuff.cpp
			gtests_json.cpp
			cJSON_test.c gtests_threadstuff.cpp)

	add_executable ( gmanticoretest ${GTESTS_SRC} )
	target_include_directories ( gmanticoretest PRIVATE .. )
	target_link_libraries ( gmanticoretest
			GTest::gmock_main
			libsphinx
			lsearchd
			${EXTRA_LIBRARIES}
			${WSREP} )


	include ( GoogleTest )
	GTEST_DISCOVER_TESTS ( gmanticoretest PROPERTIES LABELS GOOGLE )
	message ( STATUS "Discovered and added individual google-tests" )

endif ()

