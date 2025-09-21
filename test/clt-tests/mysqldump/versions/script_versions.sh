#!/bin/bash
set -e

# Latest supported versions - UPDATE THESE when adding new versions
LATEST_MARIADB="12.0"
LATEST_MYSQL="9.4"

# Function to update documentation with latest versions
update_documentation() {
    # Определяем корень репозитория
    REPO_ROOT=$(git rev-parse --show-toplevel 2>/dev/null)

    if [ -z "$REPO_ROOT" ]; then
        echo "⚠️ Not in a git repository, trying relative path"
        # Если не в git, используем относительный путь
        # Из test/clt-tests/mysqldump/versions/ до корня - 4 уровня вверх
        DOC_FILE="../../../../manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
    else
        # Используем абсолютный путь от корня репозитория
        DOC_FILE="$REPO_ROOT/manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
    fi

    if [ ! -f "$DOC_FILE" ]; then
        echo "⚠️ Documentation file not found at: $DOC_FILE"
        echo "Current directory: $(pwd)"
        echo "Looking for alternative paths..."

        # Пробуем альтернативные пути
        local alt_paths=(
            "../../../../manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
            "../../../manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
            "manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
            "/manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
        )

        for path in "${alt_paths[@]}"; do
            if [ -f "$path" ]; then
                DOC_FILE="$path"
                echo "✅ Found documentation at: $DOC_FILE"
                break
            fi
        done

        if [ ! -f "$DOC_FILE" ]; then
            echo "❌ Documentation file not found. Tried:"
            echo "  - Main path: $REPO_ROOT/manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
            for path in "${alt_paths[@]}"; do
                echo "  - $path"
            done
            echo "⚠️ Skipping documentation update"
            return 0
        fi
    fi

    echo "📝 Updating documentation at: $DOC_FILE"

    # Create backup of documentation
    cp "$DOC_FILE" "$DOC_FILE.bak"

    # Update MariaDB version in documentation
    sed -i.tmp \
        -e "s/MariaDB\( up to\| versions up to\| up to version\)\? [0-9]\+\.[0-9]\+/MariaDB up to $LATEST_MARIADB/g" \
        -e "s/mariadb-dump (up to [0-9]\+\.[0-9]\+)/mariadb-dump (up to $LATEST_MARIADB)/g" \
        "$DOC_FILE"

    # Update MySQL version in documentation
    sed -i.tmp2 \
        -e "s/MySQL\( up to\| versions up to\| up to version\)\? [0-9]\+\.[0-9]\+/MySQL up to $LATEST_MYSQL/g" \
        -e "s/mysqldump (up to [0-9]\+\.[0-9]\+)/mysqldump (up to $LATEST_MYSQL)/g" \
        "$DOC_FILE"

    # Cleanup temporary files
    rm -f "$DOC_FILE.tmp" "$DOC_FILE.tmp2"

    # Check if documentation was actually changed
    if diff -q "$DOC_FILE.bak" "$DOC_FILE" > /dev/null; then
        echo "📝 Documentation already up to date"
        rm "$DOC_FILE.bak"
    else
        echo "✅ Documentation updated: MariaDB up to $LATEST_MARIADB, MySQL up to $LATEST_MYSQL"
        echo "   Backup saved as $DOC_FILE.bak"

        # Show what changed
        echo ""
        echo "Changes made:"
        diff --unified=1 "$DOC_FILE.bak" "$DOC_FILE" | head -20 || true

        # Remove backup after showing diff
        rm "$DOC_FILE.bak"
    fi
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
        echo "Action required:"
        echo "1. Update LATEST_MARIADB variable to the new version"
        echo "2. Add new version(s) to the versions array"
        echo "3. Run the test again"
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
        echo "Action required:"
        echo "1. Update LATEST_MYSQL variable to the new version"
        echo "2. Add new version(s) to the versions array"
        echo "3. Run the test again"
        found_new=true
    fi

    if [ "$found_new" = true ]; then
        echo ""
        echo "❗ Please update the script with new versions and run again!"
        echo "❗ Documentation will be automatically updated after successful tests"
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
for version in "${versions[@]}"; do
    # Determine database type and dump command
    if [[ $version == mariadb* ]]; then
        db_type="mariadb"
        dump_command="mariadb-dump"
    else
        db_type="mysql"
        dump_command="mysqldump"
    fi

    echo "Testing version: $version"

    # Start the database container
    docker pull --platform linux/amd64 -q $version > /dev/null
    docker run --rm -d --network=test_network --platform linux/amd64 --name db-test \
        -e MYSQL_ROOT_PASSWORD=my-secret-pw $version bash -c "tail -f /dev/null" > /dev/null
    sleep 5

    # Execute dump from the database container to Manticore
    docker exec db-test $dump_command -hmanticore -P9306 manticore t > dump.sql 2> >(grep -E -v "Warning: column statistics|Warning: version string returned by server is incorrect." >&2)

    # Drop the table in Manticore to prepare for restore
    docker exec manticore mysql -h0 -P9306 -e "DROP TABLE t;"

    # Restore dump back to Manticore
    docker exec -i db-test $db_type -hmanticore -P9306 manticore < dump.sql

    # Verify restore was successful by selecting data
    docker exec db-test $db_type -hmanticore -t -P9306 -e "select * from t order by id asc limit 20;" manticore

    # Check for errors
    if [ -s dump.sql ]; then
        echo "Dump $version completed successfully"
    else
        echo "Error: dump.sql is empty for $version"
        docker stop db-test > /dev/null
        exit 1
    fi

    # Stop and remove container
    docker stop db-test > /dev/null
    rm dump.sql
done

echo "All database versions tested successfully!"

# Update documentation after successful tests
echo ""
echo "Updating documentation with latest supported versions..."
update_documentation

echo ""
echo "✅ Test completed successfully!"
echo "✅ Supported versions: MariaDB up to $LATEST_MARIADB, MySQL up to $LATEST_MYSQL"

# Ensure we exit with success
exit 0
