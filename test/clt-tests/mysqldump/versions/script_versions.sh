#!/bin/bash

# MariaDB and MySQL versions
versions=("mariadb:10.5" "mariadb:10.6" "mariadb:10.7" "mariadb:10.8" "mariadb:10.9" "mariadb:10.10" "mariadb:10.11" "mariadb:11.0" "mariadb:11.1" "mariadb:11.2" "mariadb:11.3-rc" "mariadb:latest" "mysql:5.6" "mysql:5.7" "mysql:8.0" "mysql:8.2" "mysql:latest")
# Going through all the versions
for version in "${versions[@]}"; do
    # Defining the database type
    if [[ $version == mariadb* ]]; then
        db_type="mariadb"
        dump_command="mariadb-dump"
    else
        db_type="mysql"
        dump_command="mysqldump"
    fi

    echo "Testing version: $version"

    # Start the container
    docker pull --platform linux/amd64 -q $version > /dev/null
    docker run -d --network=test_network --platform linux/amd64 --name db-test -e MYSQL_ROOT_PASSWORD=my-secret-pw $version > /dev/null

    # Waiting for start-up
    sleep 15


    # Executing mysqldump
    docker exec db-test $image -hmanticore -P9306 manticore a > dump.sql

	# Check if the mariadb executable file exists
if docker exec db-test which mariadb &>/dev/null; then
    image="mariadb"
else
    image="mysql"
fi

# Next, we use $dump_command for mysqldump and mysql
    docker exec db-test $image -hmanticore -P9306 manticore < dump.sql
    docker exec db-test $image -hmanticore -P9306 -e "SELECT * FROM a ORDER BY id DESC LIMIT 10;" manticore

    # Checking for errors
    if [ -s dump.sql ]; then
        echo "Dump $version completed successfully"
    else
        echo "Error: dump.sql is empty for $version"
    fi

    # Stopping and deleting a container
    docker stop db-test > /dev/null
    docker rm db-test > /dev/null
    rm dump.sql
done
