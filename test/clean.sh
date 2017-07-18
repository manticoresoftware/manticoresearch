#!/bin/sh

# must be run from tests dir
if test ! -f ubertest.php; then exit 0; fi

# clean subdirs
for i in test_*
do
	if test ! -f "$i/test.xml"; then continue; fi
	rm -f "$i/report.txt"
	rm -fr "$i/Conf/"
done

# clean test index data files
rm -f data/*.sp*
rm -f data/*.mvp
rm -f data/*.meta data/*.lock data/*.kill data/*.ram
rm -f data/binlog.*

# clean test runs logs
rm -f *.log
rm -f searchd.pid error.txt config.conf
rm -f guess.txt
