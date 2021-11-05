#!/bin/bash
#Run the very same test suite as runned on CI, but locally

chmod +x valgrind
cd build

#export CTEST_BUILD_CONFIGURATION=RelWithDebInfo
#ctest -V -S ../misc/ctest/memcheck.cmake
export CTEST_BUILD_CONFIGURATION=Debug
ctest -V -S ../misc/ctest/gltest.cmake
