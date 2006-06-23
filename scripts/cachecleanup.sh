#!/bin/bash


# Cleanup cache directory
# Remove all old cached files with mtime older than index mtime

# There should be your index names and full pathes to indexfiles

indexnames=( test1 test2 )
indexfiles=( /usr/local/sphinx/test1.spd /benchmarks/work/test/test2.spd )
cachedir=/tmp/cache


for element in $(seq 0 $((${#indexnames[@]} - 1)))
do                
  echo "processing index ${indexnames[$element]}"
  find "$cachedir/${indexnames[$element]}" \( ! -newer "${indexfiles[$element]}" \) -type f -print0 | xargs -0 -r rm -f

done





