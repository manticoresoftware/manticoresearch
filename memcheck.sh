#!/bin/bash
# Run memcheck.
# This script can be run on the host, or in the docker

chmod +x valgrind
mkdir -p build
cd build

[[ -z "${CACHEB}" ]] && export CACHEB=../cache
[[ ! -e "${CACHEB}" ]] && mkdir -p ${CACHEB}
[[ -z "${uid}" ]] && export uid=`id -u`
[[ -z "${gid}" ]] && export gid=`id -g`
trap "chown -R $uid:$gid ." EXIT

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

# Analysis of memory leak reports
echo "Analyzing memory leak reports..."
LEAK_FOUND=0
LEAK_PATTERNS=(definitely invalid uninitialized)

# Function to check for memory leaks in a file
check_leaks() {
    local file=$1
    local test_num=$(basename "$file" | sed -E 's/MemoryChecker\.([0-9]+)\.log/\1/')
    
    for pattern in "${LEAK_PATTERNS[@]}"; do
        if grep -i "$pattern" "$file" > /dev/null; then
            echo "$pattern DETECTED in test_$test_num"
            cat "$file"
            LEAK_FOUND=1
        fi
    done
}

# Process all memory checker log files
for log_file in Testing/Temporary/MemoryChecker.*.log; do
    if [ -f "$log_file" ]; then
        check_leaks "$log_file"
    fi
done

if [ $LEAK_FOUND -eq 1 ]; then
    echo "Memory leaks were detected. Check the logs above for details."
    exit 1
else
    echo "No memory leaks detected."
    exit 0
fi
