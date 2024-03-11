#!/bin/bash

wget https://raw.githubusercontent.com/zeraye/names-surnames-list/master/male-names-list.txt -O names.txt
wget https://raw.githubusercontent.com/zeraye/names-surnames-list/master/female-names-list.txt -O fnames.txt
cat fnames.txt >> names.txt
wget https://raw.githubusercontent.com/zeraye/names-surnames-list/master/surnames-list.txt -O surnames.txt
wget https://gist.githubusercontent.com/sanikolaev/5f4cea361b6869ee9ca7eb815be4fb76/raw/15655adc2dcb3a0c7680d07127fa50a3528a8d73/load_us_names.php -O load_us_names.php
php -d memory_limit=512G load_us_names.php 10000 10 10000000 1

# Capture the current time in nanoseconds before starting the curl operations
start=$(date +%s%N)

# Execute a curl POST request 100 times in a loop, discard the output
for n in `seq 1 100`; do curl -sX POST http://localhost:9308/search -d '{"index" : "name", "query" : {"match" : {"_all": "on"} }, "limit": 0}'; done > /dev/null

# Capture the current time in nanoseconds after completing the curl operations
end=$(date +%s%N)

# Calculate the duration of the curl operations in milliseconds
curl=$(( (end - start) / 1000000 ))

# Capture the current time in nanoseconds before starting the MySQL operations
start=$(date +%s%N)

# Execute a MySQL query 100 times in a loop, discard the output
for n in `seq 1 100`; do mysql -P9306 -h0 -e "select * from name where match('on'); show meta;"; done > /dev/null

# Capture the current time in nanoseconds after completing the MySQL operations
end=$(date +%s%N)

# Calculate the duration of the MySQL operations in milliseconds
mysql=$(( (end - start) / 1000000 ))

# Print the durations of MySQL and curl operations in milliseconds
echo "Duration: ${mysql} ${curl} ms"

# Use PHP to calculate the percentage difference between the curl and MySQL durations and check if it's >= 1%
# Exits with 0 if the condition is true, otherwise exits with 1
php -d memory_limit=512G -r "exit(round(abs($curl - $mysql) / ($curl + $mysql)/2 * 100, 0) >= 1);"

# Print the exit status of the PHP script (0 if percentage difference >= 1%, 1 otherwise)
echo $?
