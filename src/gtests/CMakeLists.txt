cmake_minimum_required ( VERSION 3.1.0 )

# Read docs at https://github.com/google/googletest.git
include ( GetGoogletest )
if ( HAVE_GOOGLETEST )

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
			gmock_main
			libsphinx
			lsearchd
			${EXTRA_LIBRARIES}
			${WSREP} )

	if ( CMAKE_VERSION VERSION_LESS 3.9.0 )# freshest cmake able to discover google-tests
		find_package ( GTest QUIET )
		GTEST_ADD_TESTS ( gmanticoretest "" ${GTESTS_SRC} )
		message ( STATUS "Added individual google-tests" )
	elseif ( CMAKE_VERSION VERSION_LESS 3.10.0 )
		include ( GoogleTest )
		GTEST_ADD_TESTS ( TARGET gmanticoretest SOURCES "${GTESTS_SRC}" TEST_LIST gTests )
		SET_TESTS_PROPERTIES ( ${gTests} PROPERTIES LABELS GOOGLE )
		message ( STATUS "Added individual google-tests" )
	else () # freshest cmake able to discover google-tests
		include ( GoogleTest )
		GTEST_DISCOVER_TESTS ( gmanticoretest PROPERTIES LABELS GOOGLE )
		message ( STATUS "Discovered and added individual google-tests" )
	endif ()

endif ( HAVE_GOOGLETEST )

