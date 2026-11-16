#
# Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
# All rights reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License. You should have
# received a copy of the GPL license along with this program; if you
# did not, you can find it at http://www.gnu.org/
#

# Keep ISA flags and LTO confined to individual object targets. In particular,
# never propagate SIMD flags to manticore_std or the daemon's other sources.
include ( CheckCXXSourceCompiles )
include ( CMakePushCheckState )
option ( WITH_BITMAP_SIMD "Build runtime-dispatched x86 bitmap kernels" ON )
add_library ( manticore_bitmap STATIC )

function ( bitmap_objects NAME )
	add_library ( bitmap_${NAME} OBJECT ${ARGN} )
	target_link_libraries ( bitmap_${NAME} PRIVATE lextra )
	set_target_properties ( bitmap_${NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE POSITION_INDEPENDENT_CODE ON )
	# Also override LTO supplied through global compiler flags.
	if ( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
		target_compile_options ( bitmap_${NAME} PRIVATE /GL- )
	elseif ( CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" )
		if ( MSVC )
			target_compile_options ( bitmap_${NAME} PRIVATE /clang:-fno-lto )
		else ()
			target_compile_options ( bitmap_${NAME} PRIVATE -fno-lto )
		endif ()
	endif ()
	target_sources ( manticore_bitmap PRIVATE $<TARGET_OBJECTS:bitmap_${NAME}> )
endfunction ()

bitmap_objects ( base bitmap_dispatch.cpp bitmap_generic.cpp )

if ( NOT WITH_BITMAP_SIMD )
	return ()
endif ()

# Check the compiler target, not the build host (also works when cross compiling).
check_cxx_source_compiles ( "
#if !defined(__x86_64__) && !defined(_M_X64)
#error Not an x86-64 target
#endif
int main() { return 0; }" BITMAP_TARGET_X64 )
if ( NOT BITMAP_TARGET_X64 )
	return ()
endif ()

function ( bitmap_variant NAME TEST_SOURCE )
	cmake_push_check_state ( RESET )
	string ( JOIN " " CMAKE_REQUIRED_FLAGS ${ARGN} )
	string ( TOUPPER "${NAME}" ISA )
	check_cxx_source_compiles ( "#include <immintrin.h>
int main() { ${TEST_SOURCE} }" BITMAP_COMPILER_${ISA} )
	cmake_pop_check_state ()
	if ( BITMAP_COMPILER_${ISA} )
		bitmap_objects ( ${NAME} bitmap_${NAME}.cpp )
		target_compile_options ( bitmap_${NAME} PRIVATE ${ARGN} )
		target_compile_definitions ( bitmap_base PRIVATE BITMAP_HAVE_${ISA}=1 )
	endif ()
endfunction ()

if ( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
	bitmap_variant ( popcnt "return int(__popcnt64(123));" )
	bitmap_variant ( ssse3 "return _mm_cvtsi128_si32(_mm_shuffle_epi8(_mm_setzero_si128(), _mm_setzero_si128()));" )
	bitmap_variant ( avx2 "return _mm256_movemask_epi8(_mm256_setzero_si256());" /arch:AVX2 )
	bitmap_variant ( avx512 "return _mm_cvtsi128_si32(_mm512_castsi512_si128(_mm512_popcnt_epi64(_mm512_setzero_si512())));" /arch:AVX512 )
elseif ( CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" )
	set ( PREFIX "" )
	if ( MSVC )
		set ( PREFIX "/clang:" )
	endif ()
	bitmap_variant ( popcnt "return __builtin_popcountll(123);" ${PREFIX}-mpopcnt )
	bitmap_variant ( ssse3 "return _mm_cvtsi128_si32(_mm_shuffle_epi8(_mm_setzero_si128(), _mm_setzero_si128()));" ${PREFIX}-mssse3 )
	bitmap_variant ( avx2 "return _mm256_movemask_epi8(_mm256_setzero_si256());" ${PREFIX}-mavx2 ${PREFIX}-mpopcnt )
	bitmap_variant ( avx512 "return _mm_cvtsi128_si32(_mm512_castsi512_si128(_mm512_popcnt_epi64(_mm512_setzero_si512())));" ${PREFIX}-mavx2 ${PREFIX}-mpopcnt ${PREFIX}-mavx512f ${PREFIX}-mavx512vpopcntdq )
endif ()
