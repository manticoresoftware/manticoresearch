cmake_minimum_required ( VERSION 3.11.0 )

# Read docs at https://github.com/google/benchmark
include ( GetGooglebench )
if (NOT TARGET benchmark::benchmark_main)
	return ()
endif ()

add_executable ( gmanticorebench
		functions.cpp
		text.cpp
		json.cpp
		cJSON_test.c
		locators.cpp
		popcount.cpp
		)

target_include_directories ( gmanticorebench PRIVATE .. )
target_link_libraries ( gmanticorebench
		lmanticore
		lsearchd
		stub_ssl
		benchmark::benchmark )

