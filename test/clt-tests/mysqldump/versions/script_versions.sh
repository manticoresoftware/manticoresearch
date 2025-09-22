#!/bin/bash
set -e

# Check for new major.minor versions
echo "🔍 Checking for new major.minor versions..."

LATEST_MARIADB="12.0"
LATEST_MYSQL="9.4"

if command -v curl >/dev/null 2>&1; then
    found_new=false

    mariadb_versions=$(curl -s "https://hub.docker.com/v2/repositories/library/mariadb/tags/?page_size=100" 2>/dev/null | grep -o '"name":"[0-9]*\.[0-9]*"' | grep -o '[0-9]*\.[0-9]*' | sort -V)
    mysql_versions=$(curl -s "https://hub.docker.com/v2/repositories/library/mysql/tags/?page_size=100" 2>/dev/null | grep -o '"name":"[0-9]*\.[0-9]*"' | grep -o '[0-9]*\.[0-9]*' | sort -V)

    newer_mariadb=$(echo "$mariadb_versions" | awk -v latest="$LATEST_MARIADB" '
    function version_compare(v1, v2) {
        split(v1, a, ".")
        split(v2, b, ".")
        if (a[1] != b[1]) return a[1] - b[1]
        return a[2] - b[2]
    }
    version_compare($0, latest) > 0 { print $0 }')

    if [ -n "$newer_mariadb" ]; then
        echo "🆕 NEW MariaDB versions detected:"
        echo "$newer_mariadb" | sed 's/^/  - mariadb:/'
        found_new=true
    fi

    newer_mysql=$(echo "$mysql_versions" | awk -v latest="$LATEST_MYSQL" '
    function version_compare(v1, v2) {
        split(v1, a, ".")
        split(v2, b, ".")
        if (a[1] != b[1]) return a[1] - b[1]
        return a[2] - b[2]
    }
    version_compare($0, latest) > 0 { print $0 }')

    if [ -n "$newer_mysql" ]; then
        echo "🆕 NEW MySQL versions detected:"
        echo "$newer_mysql" | sed 's/^/  - mysql:/'
        found_new=true
    fi

    if [ "$found_new" = false ]; then
        echo "✅ No new versions found after MariaDB $LATEST_MARIADB and MySQL $LATEST_MYSQL"
    else
        echo "❗ Please update the versions array and test new versions!"
    fi

    echo "✅ Version check completed"
else
    echo "⚠️ curl not available - skipping major version check"
fi
echo ""

# MariaDB and MySQL versions
versions=("mariadb:10.5" "mariadb:10.6" "mariadb:10.7" "mariadb:10.8" "mariadb:10.9" "mariadb:10.10" "mariadb:10.11" "mariadb:11.0" "mariadb:11.1" "mariadb:11.2" "mariadb:11.3-rc" "mariadb:11.4" "mariadb:11.5" "mariadb:11.6" "mariadb:11.7" "mariadb:11.8" "mariadb:12.0" "mariadb:latest" "mysql:5.6" "mysql:5.7" "mysql:8.0" "mysql:8.2" "mysql:8.3" "mysql:8.4" "mysql:9.0" "mysql:9.1" "mysql:9.2" "mysql:9.3" "mysql:9.4" "mysql:latest")

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
    sleep 5

    # Executing dump
    docker exec db-test $dump_command -hmanticore -P9306 manticore t > dump.sql 2> >(grep -E -v "Warning: column statistics|Warning: version string returned by server is incorrect." >&2)
    docker exec manticore mysql -h0 -P9306 -e "DROP TABLE t;"

    # Restore dump
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

echo "All database versions tested successfully!"

# ===== ДОБАВЛЕНО: Обновление документации =====
echo ""
echo "Updating documentation..."

# Пробуем найти файл документации через разные пути
DOC_FILE=""
POSSIBLE_PATHS=(
    "../../../../manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
    "$(git rev-parse --show-toplevel 2>/dev/null)/manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
)

for path in "${POSSIBLE_PATHS[@]}"; do
    if [ -f "$path" ]; then
        DOC_FILE="$path"
        break
    fi
done

if [ -f "$DOC_FILE" ]; then
    # Обновляем версии в документации
    sed -i "s/MariaDB up to [0-9]\+\.[0-9]\+/MariaDB up to $LATEST_MARIADB/g" "$DOC_FILE"
    sed -i "s/MySQL up to [0-9]\+\.[0-9]\+/MySQL up to $LATEST_MYSQL/g" "$DOC_FILE"
    echo "✅ Documentation updated: MariaDB up to $LATEST_MARIADB, MySQL up to $LATEST_MYSQL"
else
    echo "⚠️ Documentation file not found, please update manually"
    echo "   Add to documentation: MariaDB up to $LATEST_MARIADB, MySQL up to $LATEST_MYSQL"
fi
