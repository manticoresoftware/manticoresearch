#!/bin/sh

FAILLOG="/tmp/faillog1"
DIFF='smoke_diff.txt'
RES='smoke_test.txt'
REF='smoke_ref.txt'
LINE='-----------------------------\n'

die()
{
	cat $FAILLOG
	echo $LINE
	[ ! "z$2" = "z" ] && { eval $2; echo "$LINE"; }
	echo "C API:$1"
	[ -e "$FAILLOG" ] && rm $FAILLOG
	exit 1
}

cmd ()
{
        echo "Executing: $1\n">$FAILLOG
        eval $1 1>>$FAILLOG 2>&1 || die "$2" "$3"
}

cmd "./configure --with-debug" "configure failed"
cmd "make clean" "make clean failed"
cmd "make" "make failed"


cmd "../../src/indexer -c smoke_test.conf --all" "indexing failed"
cmd "../../src/searchd -c smoke_test.conf" "searchd start failed"
sleep 1
cmd "./test --smoke --port 10312>$RES" "test --smoke --port 10312 failed"
cmd "../../src/searchd -c smoke_test.conf --stop" "searchd stop failed"

cmd "make clean" " "

cmd "diff --unified=3 $REF $RES >$DIFF" 'diff failed' "cat $DIFF"

rm $RES
rm $DIFF
rm $FAILLOG

echo "all ok"
exit 0

