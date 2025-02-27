#!/bin/bash
# Run memcheck.
# This script can be run on the host, or in the docker

chmod +x valgrind
cd build

[[ ! -z "${CACHEB}" ]] && export CACHEB=../cache
[[ ! -z "${uid}" ]] && export uid=`id -u`
[[ ! -z "${gid}" ]] && export gid=`id -g`

export CTEST_CONFIGURATION_TYPE=RelWithDebInfo
export CTEST_CMAKE_GENERATOR=Ninja
export XTRA_FLAGS="CMAKE_C_COMPILER=/usr/bin/clang-16;CMAKE_CXX_COMPILER=/usr/bin/clang++-16"

export NO_TESTS=1
time ctest -VV -S ../misc/ctest/memcheck.cmake
src/searchd --mockstack | tail -n +6 > stack.txt

source stack.txt
#export MEMORYCHECK_COMMAND_OPTIONS='--tool=memcheck --leak-check=full --track-origins=yes --time-stamp=yes'
export MANTICORE_THREADS_EX='2/1+2/1'
export NO_TESTS=0
rm -f Testing/Temporary/MemoryChecker.*.log
time ctest -V -S ../misc/ctest/memcheck.cmake
find Testing/Temporary/MemoryChecker.*.log -size 0 -delete
chown -R $uid:$gid .