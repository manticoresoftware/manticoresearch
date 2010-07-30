#!/bin/sh

jobs=$(grep -c processor /proc/cpuinfo)

if [ q"1" = q"$jobs" ] ; then
    jobs=""
else
    jobs="-j$jobs"
fi

mysql -u root test < example.sql || { echo "$BANNER: documents setup failed"; exit 1; }

for CONFARGS in "--with-debug" "--with-debug --enable-id64";
do
	BANNER="testing $CONFARGS build"
	./configure $CONFARGS 1>/dev/null 2>&1 || { echo "$BANNER: configure failed"; exit 1; }
	make clean 1>/dev/null 2>&1 || { echo "$BANNER: make clean failed"; exit 1; }
	make $jobs 1>/dev/null 2>&1 || { echo "$BANNER: make failed"; exit 1; }

	cd ./test
	php ubertest.php t -u test --strict 1>/dev/null 2>&1 || { echo "$BANNER: regression suite failed"; exit 1; }
	cd ..

	cd ./src
	./tests 1>/dev/null 2>&1 || { echo "$BANNER: unit tests failed"; exit 1; }
	cd ..
	
	cd ./api/libsphinxclient/
	./smoke_test.sh || { echo "$BANNER: C API tests failed"; exit 1; }
	cd ../../
done

make clean 1>/dev/null 2>&1

echo "all ok"
exit 0
