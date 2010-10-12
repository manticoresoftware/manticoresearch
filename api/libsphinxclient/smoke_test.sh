#!/bin/sh

BANNER='C API'
LOG='smoke_log.txt'
DIFF='smoke_diff.txt'
RES='smoke_test.txt'
REF='smoke_ref.txt'

./configure --with-debug 1>$LOG 2>&1 || { echo "$BANNER: configure failed"; exit 1; }
make clean 1>$LOG 2>&1 || { echo "$BANNER: make clean failed"; exit 1; }
make 1>$LOG 2>&1 || { echo "$BANNER: make failed"; exit 1; }

../../src/indexer -c smoke_test.conf --all  1>$LOG 2>&1 || { echo "$BANNER: indexing failed"; exit 1; }
../../src/searchd -c smoke_test.conf 1>$LOG 2>&1 || { echo "$BANNER: searchd start failed"; exit 1; }
./test --smoke --port 10312 1>$RES || { exit 1; }
../../src/searchd -c smoke_test.conf --stop 1>$LOG 2>&1 || { echo "$BANNER: searchd stop failed"; exit 1; }

make clean 1>$LOG 2>&1

diff --unified=3 $REF $RES >$DIFF || { echo "$BANNER: diff failed"; exit 1; }

rm $RES
rm $DIFF
rm $LOG

echo "all ok"
exit 0
