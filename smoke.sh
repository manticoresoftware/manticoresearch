#!/bin/bash
#Run the very same test suite as runned on CI, but locally

chmod +x valgrind
cd build

# last known values (run searchd with logdebug to see the values)
export MANTICORE_KNOWN_CREATE_SIZE=16
export MANTICORE_START_KNOWN_CREATE_SIZE=2976
export MANTICORE_KNOWN_EXPR_SIZE=32
export MANTICORE_START_KNOWN_EXPR_SIZE=104
export MANTICORE_KNOWN_FILTER_SIZE=224
export MANTICORE_START_KNOWN_FILTER_SIZE=11360
export MANTICORE_KNOWN_MATCH_SIZE=336
export MANTICORE_START_KNOWN_MATCH_SIZE=14392

export CTEST_CONFIGURATION_TYPE=RelWithDebInfo
export CTEST_CMAKE_GENERATOR=Ninja
export NO_STACK_CALCULATION=1
export MANTICORE_THREADS_EX='2/1+2/1'
export XTRA_FLAGS="CMAKE_C_COMPILER=/usr/bin/clang-15;CMAKE_CXX_COMPILER=/usr/bin/clang++-15"
#export MEMORYCHECK_COMMAND_OPTIONS='--tool=memcheck --leak-check=full --track-origins=yes --time-stamp=yes'
time ctest -V -S ../misc/ctest/memcheck.cmake
#export CTEST_CONFIGURATION_TYPE=Debug
#ctest -V -S ../misc/ctest/gltest.cmake
