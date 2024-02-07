#!/bin/bash
set -e

# MariaDB and MySQL versions
versions=("mariadb:11.3-rc" "mysql:8.0")

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
    docker exec db-test $db_type -hmanticore -P9306 -e "select * from t order by id asc limit 20;" manticore

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
