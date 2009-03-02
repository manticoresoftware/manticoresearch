#!/bin/sh

for CONFARGS in "--with-debug" "--with-debug --enable-id64";
do
	BANNER="testing $CONFARGS build"
	./configure $CONFARGS 1>/dev/null 2>&1 || { echo "$BANNER: configure failed"; exit 1; }
	make clean 1>/dev/null 2>&1 || { echo "$BANNER: make clean failed"; exit 1; }
	make 1>/dev/null 2>&1 || { echo "$BANNER: make failed"; exit 1; }

	cd ./test
	php ubertest.php t -u test --strict 2>&1 || { echo "$BANNER: regression suite failed"; exit 1; }
	cd ..

	cd ./src
	./tests 2>&1 || { echo "$BANNER: unit tests failed"; exit 1; }
	cd ..
done

make clean 1>/dev/null 2>&1

echo "all ok"
exit 0
