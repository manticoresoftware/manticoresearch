#!/bin/sh

cd /manticore_test
rm -rf build
mkdir build && cd build

cmake -DPACK=1 ..
export CMAKE_TOOLCHAIN_FILE=$(pwd)/dist/build_dockers/cross/linux.cmake
cmake --build .

echo 'Done'