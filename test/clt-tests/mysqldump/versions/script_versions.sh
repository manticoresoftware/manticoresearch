#!/bin/bash
set -e

# MariaDB and MySQL versions
versions=("mariadb:10.5" "mariadb:10.6" "mariadb:10.7" "mariadb:10.8" "mariadb:10.9" "mariadb:10.10" "mariadb:10.11" "mariadb:11.0" "mariadb:11.1" "mariadb:11.2" "mariadb:11.3-rc" "mariadb:11.4" "mariadb:11.5" "mariadb:11.6" "mariadb:11.7" "mariadb:latest" "mysql:5.6" "mysql:5.7" "mysql:8.0" "mysql:8.2" "mysql:8.3" "mysql:8.4" "mysql:9.0" "mysql:9.1" "mysql:9.2" "mysql:latest")

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
    docker run --rm -d --network=test_network --platform linux/amd64 --name db-test -e MYSQL_ROOT_PASSWORD=my-secret-pw $version bash -c "tail -f /dev/null" > /dev/null
    sleep 1

    # Executing mysqldump
    docker exec db-test $dump_command -hmanticore -P9306 manticore t > dump.sql
	docker exec manticore mysql -h0 -P9306 -e "DROP TABLE t;"

    # Next, we use $dump_command for mysqldump and mysql
    docker exec -i db-test $db_type -hmanticore -P9306 manticore < dump.sql
    docker exec db-test $db_type -hmanticore -t -P9306 -e "select * from t order by id asc limit 20;" manticore

    # Checking for errors
    if [ -s dump.sql ]; then
        echo "Dump $version completed successfully"
    else
        echo "Error: dump.sql is empty for $version"
    fi

    # Stopping and deleting a container
    docker stop db-test > /dev/null
    rm dump.sql
done
