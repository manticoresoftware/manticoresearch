#!/bin/sh

jobs=$(grep -c processor /proc/cpuinfo)
[ "z$FAILLOG" = "z" ] && FAILLOG="/tmp/faillog"
LINE="-------------------------------\n"
if [ q"1" = q"$jobs" ] ; then
    jobs=""
else
    jobs="-j$jobs"
fi

die()
{
	cat $FAILLOG
	echo $LINE
	[ ! "z$2" = "z" ] && { eval $2; echo "$LINE"; }
	echo "$1"
	[ -e "$FAILLOG" ] && rm $FAILLOG
	exit 1
}


cmd1 ()
{
	echo "Executing: $2\n">$FAILLOG
	eval $2 1>>$1 2>&1 || die "$3" "$4"
}

cmd ()
{
	cmd1 "$FAILLOG" "$1" "$2" "$3" 
}

cmd "mysql -u root test < example.sql" "Documents setup failed"

for CONFARGS in "--with-debug" "--with-debug --enable-id64";
do
	BANNER="testing $CONFARGS build"
	cmd "./configure $CONFARGS" "$BANNER: configure failed"
	cmd "make clean" "$BANNER: make clean failed" 
	cmd "make $jobs" "$BANNER: make failed" 

	cmd1 "/dev/null" "cd ./test; php ubertest.php t -u test --strict-verbose --no-demo" "$BANNER: regression suite failed" "cat report.txt; rm report.txt"
	cd ..

	cmd "cd ./src; ./tests" "$BANNER: unit tests failed" 
	cd ..
	
	cmd "cd ./api/libsphinxclient; ./smoke_test.sh" "$BANNER: C API tests failed"
	cd ../../ 
done

make clean 1>/dev/null 2>&1
[ -e "$FAILLOG" ] && rm "$FAILLOG"

echo "all ok\n"
exit 0
