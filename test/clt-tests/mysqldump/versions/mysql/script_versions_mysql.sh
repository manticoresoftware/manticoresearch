#!/bin/bash
set -e

# Check for new major.minor versions
echo "🔍 Checking for new MySQL major.minor versions..."

LATEST_MYSQL="9.6"

if command -v curl >/dev/null 2>&1; then
    found_new=false

    mysql_versions=$(curl -s "https://hub.docker.com/v2/repositories/library/mysql/tags/?page_size=100" 2>/dev/null | grep -o '"name":"[0-9]*\.[0-9]*"' | grep -o '[0-9]*\.[0-9]*' | sort -V)

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
        echo "✅ No new versions found after MySQL $LATEST_MYSQL"
    else
        echo "❗ Please update the versions array and test new versions!"
        echo "❗ Also update:"
        echo "   - LATEST_MYSQL variable in this script"
        echo "   - Version numbers in documentation (manual/english/Securing_and_compacting_a_table/Backup_and_restore.md)"
        exit 1
    fi

    echo "✅ Version check completed"
else
    echo "⚠️ curl not available - skipping major version check"
fi
echo ""

# MySQL versions
versions=("mysql:5.6" "mysql:5.7" "mysql:8.0" "mysql:8.2" "mysql:8.3" "mysql:8.4" "mysql:9.0" "mysql:9.1" "mysql:9.2" "mysql:9.3" "mysql:9.4" "mysql:9.5" "mysql:9.6" "mysql:latest")

# Going through all the versions
for version in "${versions[@]}"; do
    db_type="mysql"
    dump_command="mysqldump"

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
    restore_status=$?
    
    # Clear query log before test query
    docker exec manticore bash -c "> /var/log/manticore/query.log" 2>/dev/null || true
    
    # Execute test query (only if restore succeeded)
    if [ $restore_status -eq 0 ]; then
        docker exec db-test $db_type -hmanticore -t -P9306 -e "select * from t order by id asc limit 20;" manticore
    else
        echo "⚠️ Warning: Restore failed for $version, skipping query log check"
    fi
    
    # Output query log content (only if restore succeeded) - CLT will validate against expected output
    if [ $restore_status -eq 0 ]; then
        echo "   Query log content:"
        docker exec manticore cat /var/log/manticore/query.log 2>/dev/null || echo "   (empty)"
    fi

    # Checking for errors
    if [ -s dump.sql ]; then
        echo "Dump $version completed successfully"
    else
        echo "Error: dump.sql is empty for $version"
    fi

    # Stopping and deleting a container
    docker stop db-test > /dev/null
    rm -f dump.sql
done

echo "All MySQL versions tested successfully!"

# Check documentation versions
echo ""
echo "Checking documentation versions..."

# Check documentation from mounted /manual volume
DOC_FILE="/manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"

if [ -f "$DOC_FILE" ]; then
    echo "Checking documentation file..."

    # Extract versions from documentation
    DOC_MYSQL=$(grep -o "MySQL up to [0-9]\+\.[0-9]\+" "$DOC_FILE" | grep -o "[0-9]\+\.[0-9]\+" | head -1)

    echo "Script version: MySQL $LATEST_MYSQL"
    echo "Documentation version: MySQL ${DOC_MYSQL:-not found}"

    # Check if they match
    if [ "$DOC_MYSQL" = "$LATEST_MYSQL" ]; then
        echo "✅ Documentation MySQL version matches script version"
    else
        echo "❌ Documentation MySQL version doesn't match script version!"
        echo ""
        echo "Script has: MySQL $LATEST_MYSQL"
        echo "Documentation has: MySQL ${DOC_MYSQL:-not found}"
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
