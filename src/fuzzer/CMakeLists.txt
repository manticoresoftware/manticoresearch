cmake_minimum_required ( VERSION 3.20 )

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	return()
endif()

if (NOT QFUZZER)
	return()
endif()

add_executable ( qfuzzer EXCLUDE_FROM_ALL fuzzer.cpp )
target_include_directories ( qfuzzer PRIVATE "${MANTICORE_SOURCE_DIR}/src" )
target_link_libraries ( qfuzzer PRIVATE lextra lmanticore sphinxquery -fsanitize=address,fuzzer )
message (STATUS "Target qfuzzer added")