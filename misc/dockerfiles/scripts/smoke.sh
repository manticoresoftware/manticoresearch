#!/bin/sh

#for CONFARGS in "--with-debug" "--with-debug --disable-id64" "--with-debug --with-unixodbc";
CONFARGS="--with-debug"
RT=""

while getopts "c:t:" option; do
  case $option in
	c) CONFARGS="--with-debug --"$OPTARG ;;
    t) RT="--"$OPTARG ;;
  esac
done

# remove the options from the positional parameters
shift $(( OPTIND - 1 ))

jobs=$(grep -c processor /proc/cpuinfo)
[ "z$FAILLOG" = "z" ] && FAILLOG="/tmp/faillog"
LINE="-------------------------------\n"
if [ q"1" = q"$jobs" ] ; then
    jobs=""
else
    jobs="-j$jobs"
fi

echo "configure '"$CONFARGS"'"
echo "jobs '"$jobs"'"
echo "test '"$RT"'"

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

cmd "cp -R /search_src/* /search/"
cmd "cd /search/"

echo "Waiting for db instance"
until mysql -h db -utest -p"$MYSQL_PASSWORD" &> /dev/null
do
  printf "."
  sleep 1
done

echo "\t mysql ready"

cat >/root/.sphinx <<EOL
(
"db-host"=>"db",
"db-user"=>"test",
"db-password"=>"$MYSQL_PASSWORD",
"data"=>"data",
"lemmatizer_base"=>"/aot",
'odbc_driver'=>'MyODBC'
)
EOL

cmd "mysql -h db -utest -p"$MYSQL_PASSWORD" test < example.sql" "Documents setup failed. Log in into mysql as admin and perform: CREATE DATABASE test; CREATE USER test@localhost; GRANT ALL PRIVILEGES ON test.* TO test@localhost;"

BANNER="testing $CONFARGS build"
./configure $CONFARGS
make clean
make $jobs

cd ./test
php ubertest.php t --keep-all $RT

cd ..

cd ./src
./tests
cd ..

cd ./api/libsphinxclient
./smoke_test.sh
cd ../../ 

zip result.zip ./test/test_*/report.txt ./test/error.txt ./api/libsphinxclient/smoke_diff.txt

make clean 1>/dev/null 2>&1
[ -e "$FAILLOG" ] && rm "$FAILLOG"

echo "all ok\n"
exit 0
