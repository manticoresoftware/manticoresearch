#!/bin/sh

# must be run from tests dir
if test ! -f ubertest.php; then exit 0; fi

# not suitable for in-source run
if test ! -f CTestTestfile.cmake; then exit 0; fi

# clean test index data files
rm -rf data*
rm -rf test_*
rm -rf rt_*
rm -rf indextool*

# clean test runs logs
rm error*
rm -f *.log
rm -f config*
rm -f searchd.pid
rm -f guess.txt report.txt examples.txt

