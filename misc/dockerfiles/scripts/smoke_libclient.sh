#!/bin/sh

#for CONFARGS in "--with-debug" "--with-debug --disable-id64" "--with-debug --with-unixodbc";
CONFARGS="--with-debug"
RT=""
RESFILE="result.zip"

while getopts "c:t:o:" option; do
  case $option in
	c) CONFARGS="--with-debug --"$OPTARG ;;
    t) RT="--"$OPTARG ;;
	o) RESFILE=$OPTARG ;;
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
echo "result '"$RESFILE"'"

die()
{
	cat $FAILLOG
	echo $LINE
	[ ! "z$2" = "z" ] && { eval $2; echo "$LINE"; }
	echo "$1"
	[ -e "$FAILLOG" ] && rm $FAILLOG
	exit 1
}

cp -R /search_src/* /search/
cd /search/

until nc -z -w30 db 3306
do
  printf "."
  # wait for 5 seconds before check again
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

mysql -h db -utest -p"$MYSQL_PASSWORD" test < example.sql

./configure $CONFARGS
make clean
make $jobs

cd ./api/libsphinxclient
./smoke_test.sh
exit_code=$?

cd /search/
zip -q /search_src/$RESFILE ./api/libsphinxclient/smoke_diff.txt

exit $exit_code
