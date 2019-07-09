#!/bin/bash
#Run the very same test suite as runned on CI, but locally

export CTEST_BUILD_CONFIGURATION=Debug
ctest -V -S misc/ctest/gltest.cmake