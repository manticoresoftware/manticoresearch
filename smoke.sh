#!/bin/bash
#Run the very same test suite as runned on CI, but locally

chmod +x valgrind
cd build

export CTEST_CONFIGURATION_TYPE=RelWithDebInfo
export CTEST_CMAKE_GENERATOR=Ninja
export XTRA_FLAGS="CMAKE_C_COMPILER=/usr/bin/clang-12;CMAKE_CXX_COMPILER=/usr/bin/clang++-12"
#export MEMORYCHECK_COMMAND_OPTIONS='--tool=memcheck --leak-check=full --track-origins=yes --time-stamp=yes'
ctest -V -S ../misc/ctest/memcheck.cmake
#export CTEST_CONFIGURATION_TYPE=Debug
#ctest -V -S ../misc/ctest/gltest.cmake
