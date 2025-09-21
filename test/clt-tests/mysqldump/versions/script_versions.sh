#!/bin/bash
set -e

# Latest supported versions - UPDATE THESE when adding new versions
LATEST_MARIADB="12.0"
LATEST_MYSQL="9.4"

# Function to update documentation with latest versions
update_documentation() {
    # Temporarily disable set -e for this function
    set +e

    echo "Starting documentation update..."

    # Determine repository root
    REPO_ROOT=$(git rev-parse --show-toplevel 2>/dev/null)

    if [ -z "$REPO_ROOT" ]; then
        echo "⚠️ Not in a git repository, trying relative path"
        DOC_FILE="../../../../manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
    else
        DOC_FILE="$REPO_ROOT/manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
    fi

    if [ ! -f "$DOC_FILE" ]; then
        echo "⚠️ Documentation file not found"
        echo "⚠️ Documentation update skipped"
        set -e
        return 0
    fi

    echo "📝 Updating documentation..."

    # Create backup
    cp "$DOC_FILE" "$DOC_FILE.bak" 2>/dev/null || true

    # Detect OS and use appropriate sed syntax
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        sed -i '' \
            -e "s/MariaDB\( up to\| versions up to\| up to version\)\? [0-9]\+\.[0-9]\+/MariaDB up to $LATEST_MARIADB/g" \
            -e "s/mariadb-dump (up to [0-9]\+\.[0-9]\+)/mariadb-dump (up to $LATEST_MARIADB)/g" \
            "$DOC_FILE" 2>/dev/null || true

        sed -i '' \
            -e "s/MySQL\( up to\| versions up to\| up to version\)\? [0-9]\+\.[0-9]\+/MySQL up to $LATEST_MYSQL/g" \
            -e "s/mysqldump (up to [0-9]\+\.[0-9]\+)/mysqldump (up to $LATEST_MYSQL)/g" \
            "$DOC_FILE" 2>/dev/null || true
    else
        # Linux
        sed -i \
            -e "s/MariaDB\( up to\| versions up to\| up to version\)\? [0-9]\+\.[0-9]\+/MariaDB up to $LATEST_MARIADB/g" \
            -e "s/mariadb-dump (up to [0-9]\+\.[0-9]\+)/mariadb-dump (up to $LATEST_MARIADB)/g" \
            "$DOC_FILE" 2>/dev/null || true

        sed -i \
            -e "s/MySQL\( up to\| versions up to\| up to version\)\? [0-9]\+\.[0-9]\+/MySQL up to $LATEST_MYSQL/g" \
            -e "s/mysqldump (up to [0-9]\+\.[0-9]\+)/mysqldump (up to $LATEST_MYSQL)/g" \
            "$DOC_FILE" 2>/dev/null || true
    fi

    echo "✅ Documentation update completed"
    rm -f "$DOC_FILE.bak"

    # Re-enable set -e
    set -e
    return 0
}

# Check for new major.minor versions on Docker Hub
echo "🔍 Checking for new major.minor versions..."

if command -v curl >/dev/null 2>&1; then
    found_new=false

    # Fetch available versions from Docker Hub
    mariadb_versions=$(curl -s "https://hub.docker.com/v2/repositories/library/mariadb/tags/?page_size=100" 2>/dev/null | grep -o '"name":"[0-9]*\.[0-9]*"' | grep -o '[0-9]*\.[0-9]*' | sort -V)
    mysql_versions=$(curl -s "https://hub.docker.com/v2/repositories/library/mysql/tags/?page_size=100" 2>/dev/null | grep -o '"name":"[0-9]*\.[0-9]*"' | grep -o '[0-9]*\.[0-9]*' | sort -V)

    # Check for newer MariaDB versions
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
        echo ""
        echo "Action required: Update LATEST_MARIADB and versions array"
        found_new=true
    fi

    # Check for newer MySQL versions
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
        echo ""
        echo "Action required: Update LATEST_MYSQL and versions array"
        found_new=true
    fi

    if [ "$found_new" = true ]; then
        echo ""
        echo "❗ Please update the script with new versions!"
        exit 1
    else
        echo "✅ No new versions found after MariaDB $LATEST_MARIADB and MySQL $LATEST_MYSQL"
    fi

    echo "✅ Version check completed"
else
    echo "⚠️ curl not available - skipping major version check"
fi
echo ""

# Array of versions to test - ADD NEW VERSIONS HERE
versions=(
    # MariaDB versions
    "mariadb:10.5" "mariadb:10.6" "mariadb:10.7" "mariadb:10.8"
    "mariadb:10.9" "mariadb:10.10" "mariadb:10.11"
    "mariadb:11.0" "mariadb:11.1" "mariadb:11.2" "mariadb:11.3-rc"
    "mariadb:11.4" "mariadb:11.5" "mariadb:11.6" "mariadb:11.7"
    "mariadb:11.8" "mariadb:12.0"
    "mariadb:latest"
    # MySQL versions
    "mysql:5.6" "mysql:5.7"
    "mysql:8.0" "mysql:8.2" "mysql:8.3" "mysql:8.4"
    "mysql:9.0" "mysql:9.1" "mysql:9.2" "mysql:9.3" "mysql:9.4"
    "mysql:latest"
)

# Test all versions
echo "Testing ${#versions[@]} database versions..."
echo ""

for version in "${versions[@]}"; do
    # Determine database type and dump command
    if [[ $version == mariadb* ]]; then
        db_type="mariadb"
        dump_command="mariadb-dump"
    else
        db_type="mysql"
        dump_command="mysqldump"
    fi

    echo -n "Testing $version... "

    # Start the database container silently
    docker pull --platform linux/amd64 -q $version > /dev/null 2>&1
    docker run --rm -d --network=test_network --platform linux/amd64 --name db-test \
        -e MYSQL_ROOT_PASSWORD=my-secret-pw $version bash -c "tail -f /dev/null" > /dev/null 2>&1
    sleep 5

    # Execute dump from the database container to Manticore
    docker exec db-test $dump_command -hmanticore -P9306 manticore t > dump.sql 2> >(grep -E -v "Warning: column statistics|Warning: version string returned by server is incorrect." >&2)

    # Drop the table in Manticore to prepare for restore
    docker exec manticore mysql -h0 -P9306 -e "DROP TABLE t;" > /dev/null 2>&1

    # Restore dump back to Manticore
    docker exec -i db-test $db_type -hmanticore -P9306 manticore < dump.sql > /dev/null 2>&1

    # Verify restore was successful by counting rows
    ROW_COUNT=$(docker exec db-test $db_type -hmanticore -P9306 -Ne "SELECT COUNT(*) FROM t;" manticore 2>/dev/null)

    # Check for errors
    if [ -s dump.sql ] && [ "$ROW_COUNT" = "20" ]; then
        echo "✅ Success"
    else
        echo "❌ Failed (expected 20 rows, got $ROW_COUNT)"
        docker stop db-test > /dev/null 2>&1
        exit 1
    fi

    # Stop and remove container silently
    docker stop db-test > /dev/null 2>&1
    rm -f dump.sql
done

echo ""
echo "All database versions tested successfully!"

# Update documentation after successful tests
echo ""
echo "Updating documentation with latest supported versions..."
update_documentation || true

echo ""
echo "✅ Test completed successfully!"
echo "✅ Supported versions: MariaDB up to $LATEST_MARIADB, MySQL up to $LATEST_MYSQL"

# Ensure we exit with success
exit 0
