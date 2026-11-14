# Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
# All rights reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License. You should have
# received a copy of the GPL license along with this program; if you
# did not, you can find it at http://www.gnu.org/

cmake_minimum_required ( VERSION 3.17 )
set ( POSTINGS_STANDALONE OFF )
if ( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
	project ( postings_primitives LANGUAGES CXX )
	set ( POSTINGS_STANDALONE ON )
	enable_testing ()
endif ()

option ( WITH_POSTINGS_SIMD "Build baseline x86-64 SSE2 posting packing kernels" ON )
option ( WITH_POSTINGS_TOOLS "Build posting codec tests and benchmark" ${POSTINGS_STANDALONE} )
add_library ( manticore_postings STATIC codec.cpp packing.cpp stream.cpp streamvalidation.cpp )
set_source_files_properties ( streamvalidation.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON )
target_compile_features ( manticore_postings PUBLIC cxx_std_17 )
target_include_directories ( manticore_postings PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
set_target_properties ( manticore_postings PROPERTIES POSITION_INDEPENDENT_CODE ON )

include ( CheckCXXSourceCompiles )
check_cxx_source_compiles ( "
#if !defined(__x86_64__) && !defined(_M_X64)
#error Not x86-64
#endif
int main() { return 0; }" POSTINGS_TARGET_X64 )

if ( WITH_POSTINGS_SIMD AND POSTINGS_TARGET_X64 )
	add_library ( postings_sse2 OBJECT packing_sse2.cpp fastpfor/simdbitpacking.cpp )
	target_compile_features ( postings_sse2 PRIVATE cxx_std_17 )
	set_target_properties ( postings_sse2 PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE POSITION_INDEPENDENT_CODE ON UNITY_BUILD OFF )
	if ( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
		target_compile_options ( postings_sse2 PRIVATE /GL- )
	elseif ( CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT MSVC )
		target_compile_options ( postings_sse2 PRIVATE -march=x86-64 -msse2 -mno-avx -fno-lto )
	endif ()
	target_compile_definitions ( manticore_postings PRIVATE POSTINGS_HAVE_SSE2=1 )
	target_sources ( manticore_postings PRIVATE $<TARGET_OBJECTS:postings_sse2> )
endif ()

if ( WITH_POSTINGS_TOOLS )
	add_executable ( postings_stream_test tests/stream_test.cpp )
	target_link_libraries ( postings_stream_test PRIVATE manticore_postings )
	add_test ( NAME postings_stream COMMAND postings_stream_test )
	add_executable ( postings_codec_test tests/codec_test.cpp )
	target_link_libraries ( postings_codec_test PRIVATE manticore_postings )
	add_test ( NAME postings_codec COMMAND postings_codec_test )
	add_executable ( postings_codec_bench tests/codec_bench.cpp )
	target_link_libraries ( postings_codec_bench PRIVATE manticore_postings )
endif ()

if ( WITH_POSTINGS_TOOLS AND NOT POSTINGS_STANDALONE )
	add_executable ( postings_qword_test tests/qword_test.cpp )
	target_include_directories ( postings_qword_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/.." )
	target_link_libraries ( postings_qword_test PRIVATE lmanticore )
	add_test ( NAME postings_qword COMMAND postings_qword_test "${CMAKE_CURRENT_BINARY_DIR}/qword-test.spd" )
endif ()
