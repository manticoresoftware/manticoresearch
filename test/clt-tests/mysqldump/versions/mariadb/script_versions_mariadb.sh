#!/bin/bash
set -e

# Check for new major.minor versions
echo "🔍 Checking for new MariaDB major.minor versions..."

LATEST_MARIADB="12.1"

if command -v curl >/dev/null 2>&1; then
    found_new=false

    mariadb_versions=$(curl -s "https://hub.docker.com/v2/repositories/library/mariadb/tags/?page_size=100" 2>/dev/null | grep -o '"name":"[0-9]*\.[0-9]*"' | grep -o '[0-9]*\.[0-9]*' | sort -V)

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

    if [ "$found_new" = false ]; then
        echo "✅ No new versions found after MariaDB $LATEST_MARIADB"
    else
        echo "❗ Please update the versions array and test new versions!"
        echo "❗ Also update:"
        echo "   - LATEST_MARIADB variable in this script"
        echo "   - Version numbers in documentation (manual/english/Securing_and_compacting_a_table/Backup_and_restore.md)"
        exit 1
    fi

    echo "✅ Version check completed"
else
    echo "⚠️ curl not available - skipping major version check"
fi
echo ""

# MariaDB versions
versions=("mariadb:10.5" "mariadb:10.6" "mariadb:10.7" "mariadb:10.8" "mariadb:10.9" "mariadb:10.10" "mariadb:10.11" "mariadb:11.0" "mariadb:11.1" "mariadb:11.2" "mariadb:11.3-rc" "mariadb:11.4" "mariadb:11.5" "mariadb:11.6" "mariadb:11.7" "mariadb:11.8" "mariadb:12.0" "mariadb:12.1" "mariadb:latest")

# Going through all the versions
for version in "${versions[@]}"; do
    db_type="mariadb"
    dump_command="mariadb-dump"

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

echo "All MariaDB versions tested successfully!"

# Check documentation versions
echo ""
echo "Checking documentation versions..."

# Check documentation from mounted /manual volume
DOC_FILE="/manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"

if [ -f "$DOC_FILE" ]; then
    echo "Checking documentation file..."

    # Extract versions from documentation
    DOC_MARIADB=$(grep -o "MariaDB up to [0-9]\+\.[0-9]\+" "$DOC_FILE" | grep -o "[0-9]\+\.[0-9]\+" | head -1)

    echo "Script version: MariaDB $LATEST_MARIADB"
    echo "Documentation version: MariaDB ${DOC_MARIADB:-not found}"

    # Check if they match
    if [ "$DOC_MARIADB" = "$LATEST_MARIADB" ]; then
        echo "✅ Documentation MariaDB version matches script version"
    else
        echo "❌ Documentation MariaDB version doesn't match script version!"
        echo ""
        echo "Script has: MariaDB $LATEST_MARIADB"
        echo "Documentation has: MariaDB ${DOC_MARIADB:-not found}"
        echo ""
        echo "Please update documentation file:"
        echo "manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
        exit 1
    fi
else
    echo "⚠️ Documentation file not found at $DOC_FILE"
    echo "Make sure manual directory is mounted with -v"
    exit 1
fi

exit 0
