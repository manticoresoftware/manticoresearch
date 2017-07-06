#!/bin/bash

# cache directory cleanup script example
#
# removes all old cached files with mtime older than index mtime
#
# there MUST be your ACTUAL index names and FULL PATHS to indexfiles

indexnames=( test1 test2 )
indexfiles=( /usr/local/sphinx/test1.spd /benchmarks/work/test/test2.spd )
cachedir=/tmp/cache

for element in $(seq 0 $((${#indexnames[@]} - 1)))
do                
	echo "processing index ${indexnames[$element]}"
	find "$cachedir/${indexnames[$element]}" \( ! -newer "${indexfiles[$element]}" \) -type f -print0 | xargs -0 -r rm -f
done
