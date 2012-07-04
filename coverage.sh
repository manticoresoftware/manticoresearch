#!/bin/bash

# cleanup from prevous runs, if any
rm src/*.gc*
rm .coverage.*
make clean

# build
CXXFLAGS="-fprofile-arcs -ftest-coverage" ./configure --with-debug
make -j4

# initial coverage data
lcov -c -i -d src/ -o .coverage.base

# run tests
cd test
php ubertest.php t
cd ..
./src/tests

# generate coverage report
lcov -c -d src/ -o .coverage.run
lcov -d src/ -a .coverage.base -a .coverage.run -o .coverage.total
genhtml -o lcov/ .coverage.total

# cleanup
rm src/*.gc*
rm .coverage.*
