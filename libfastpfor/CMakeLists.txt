# This code is released under the
# Apache License Version 2.0 http://www.apache.org/licenses/.
#
# Copyright (c) 2012 Louis Dionne
#
cmake_minimum_required(VERSION 3.0)
project(FastPFOR CXX C)
set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")
include(AppendCompilerFlags)
include(DetectCPUFeatures)

#
# Runs compiler with "-dumpversion" and parses major/minor
# version with a regex.
#
# Taken&Modified from Boost.cmake
#
function(CXX_COMPILER_DUMPVERSION _OUTPUT_VERSION)

  exec_program(${CMAKE_CXX_COMPILER}
    ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion
    OUTPUT_VARIABLE COMPILER_VERSION
  )
  #string(REGEX REPLACE "([0-9])\\.([0-9])(\\.[0-9])?" "\\1\\2"
  #   COMPILER_VERSION ${COMPILER_VERSION})

  set(${_OUTPUT_VERSION} ${COMPILER_VERSION} PARENT_SCOPE)
endfunction()

if(NOT WIN32)
    CXX_COMPILER_DUMPVERSION(CXX_COMPILER_VERSION)
endif()

MESSAGE( STATUS "CMAKE_SIZEOF_VOID_P (should be 8): " ${CMAKE_SIZEOF_VOID_P} )
if ( NOT CMAKE_SIZEOF_VOID_P EQUAL 8 )
   MESSAGE( STATUS "Please use a 64-bit system. " )
endif()
if( SUPPORT_SSE42 )
    MESSAGE( STATUS "SSE 4.2 support detected" )
else()
    MESSAGE( STATUS "SSE 4.2 support not detected" )
endif()

set ( FastPFOR_SRCS
    src/bitpacking.cpp
    src/bitpackingaligned.cpp
    src/bitpackingunaligned.cpp
#    src/horizontalbitpacking.cpp
    src/simdunalignedbitpacking.cpp
    src/simdbitpacking.cpp
    src/varintdecode.c
    src/streamvbyte.c
)

set ( FastPFOR_HEADERS
    headers/bitpacking.h
    headers/bitpackingaligned.h
    headers/bitpackinghelpers.h
    headers/bitpackingunaligned.h
    headers/blockpacking.h
    headers/codecfactory.h
    headers/codecs.h
    headers/common.h
    headers/compositecodec.h
    headers/cpubenchmark.h
    headers/csv.h
    headers/deltautil.h
    headers/entropy.h
    headers/externalvector.h
    headers/fastpfor.h
    headers/horizontalbitpacking.h
    headers/maropuparser.h
    headers/memutil.h
    headers/mersenne.h
    headers/newpfor.h
    headers/optpfor.h
    headers/packingvectors.h
    headers/pfor.h
    headers/pfor2008.h
    headers/rolledbitpacking.h
    headers/simdbinarypacking.h
    headers/simdbitpacking.h
    headers/simdfastpfor.h
    headers/simdgroupsimple.h
    headers/simdnewpfor.h
    headers/simdoptpfor.h
    headers/simdpfor.h
    headers/simdvariablebyte.h
    headers/simple8b.h
    headers/simple8b_rle.h
    headers/simple9.h
    headers/simple9_rle.h
    headers/simple16.h
    headers/snappydelta.h
    headers/streamvariablebyte.h
    headers/stringutil.h
    headers/synthetic.h
    headers/usimdbitpacking.h
    headers/util.h
    headers/variablebyte.h
    headers/VarIntG8IU.h
    headers/varintgb.h
    headers/vsencoding.h
    headers/ztimer.h
)

add_library ( FastPFOR STATIC ${FastPFOR_SRCS} ${FastPFOR_HEADERS} )
target_include_directories(FastPFOR PUBLIC headers)

if (WIN32)
	target_compile_options(FastPFOR PRIVATE "-wd4267")
endif()

set_target_properties(FastPFOR PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    # require at least gcc 4.7
    if (CXX_COMPILER_VERSION VERSION_LESS 4.7)
        message(STATUS "GCC version must be at least 4.7!")
    endif()
    # Uncomment the following lines to see how the code compiles without AVX,SSE4.2 and/or SSE2
    #set (CMAKE_CXX_FLAGS_RELEASE "-Wall -Ofast -lm  -DNDEBUG -std=c++11 -DHAVE_CXX0X -march=x86-64")
    #set (CMAKE_CXX_FLAGS_RELEASE "-Wall -Ofast -lm  -DNDEBUG -std=c++11 -DHAVE_CXX0X -march=core2")
    #set (CMAKE_CXX_FLAGS_RELEASE "-Wall -Ofast -lm  -DNDEBUG -std=c++11 -DHAVE_CXX0X -msse4.2")

    #set (CMAKE_CXX_FLAGS_RELEASE "-Wall -Wcast-align -Ofast -lm  -DNDEBUG -std=c++11 -DHAVE_CXX0X -msse41")
    #set (CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wcast-align -ggdb  -lm  -std=c++11 -DHAVE_CXX0X -msse41")
    #set (CMAKE_C_FLAGS_RELEASE "-Wall -Wcast-align -Ofast -lm  -DNDEBUG -std=c99  -msse41")
    #set (CMAKE_C_FLAGS_DEBUG   "-Wall -Wcast-align -ggdb  -lm  -std=c99 -msse41")

	target_compile_options ( FastPFOR PRIVATE "-msse4.1")

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
    if (CXX_COMPILER_VERSION VERSION_LESS 14.0.1)
        message(STATUS "Intel version must be at least 14.0.1!")
    endif()
    set (CMAKE_CXX_FLAGS_RELEASE "-Wall -Ofast  -DNDEBUG -std=c++11 -DHAVE_CXX0X  -march=native")
    set (CMAKE_CXX_FLAGS_DEBUG   "-Wall -ggdb   -std=c++11 -DHAVE_CXX0X  -march=native")
    set (CMAKE_C_FLAGS_RELEASE "-Wall -Ofast  -DNDEBUG -std=c99  -march=native")
    set (CMAKE_C_FLAGS_DEBUG   "-Wall -ggdb   -std=c99  -march=native")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
    if (CXX_COMPILER_VERSION VERSION_LESS 4.2.1)
        message(STATUS  "Clang version must be at least 4.2.1!" )
    endif()
    #set (CMAKE_CXX_FLAGS_RELEASE "-Wall -Wcast-align -O3 -DNDEBUG -std=c++11 -DHAVE_CXX0X -msse4.1 -march=native")
    #set (CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wcast-align -ggdb  -std=c++11 -DHAVE_CXX0X -msse4.1 -march=native")
    #set (CMAKE_C_FLAGS_RELEASE "-Wall -Wcast-align -O3 -DNDEBUG -std=c99 -msse4.1 -march=native")
    #set (CMAKE_C_FLAGS_DEBUG   "-Wall -Wcast-align -ggdb  -std=c99 -msse4.1 -march=native")

	target_compile_options ( FastPFOR PRIVATE "-msse4.1")
elseif(WIN32)
    # TODO check MSVC versions?
else ()
    message(FATAL_ERROR "Please, use MSVC, GCC, Clang, or the Intel compiler!")
endif()
