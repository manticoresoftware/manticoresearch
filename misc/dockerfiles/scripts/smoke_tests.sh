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

cat >/root/.sphinx <<EOL
(
"data"=>"data",
"lemmatizer_base"=>"/aot",
)
EOL

./configure $CONFARGS
make clean
make $jobs

cd ./src
./tests
exit_code=$?

exit $exit_code
