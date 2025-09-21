#!/bin/bash
set -e

# Latest supported versions - UPDATE THESE when adding new versions
LATEST_MARIADB="12.0"
LATEST_MYSQL="9.4"

# Function to update documentation with latest versions
update_documentation() {
    # Temporarily disable set -e
    set +e

    # Find repository root
    REPO_ROOT=$(git rev-parse --show-toplevel 2>/dev/null)

    if [ -z "$REPO_ROOT" ]; then
        DOC_FILE="../../../../manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
    else
        DOC_FILE="$REPO_ROOT/manual/english/Securing_and_compacting_a_table/Backup_and_restore.md"
    fi

    if [ ! -f "$DOC_FILE" ]; then
        echo "⚠️ Doc update skipped (not found)"
        set -e
        return 0
    fi

    # Create backup silently
    cp "$DOC_FILE" "$DOC_FILE.bak" 2>/dev/null || true

    # Update versions in doc
    if [[ "$OSTYPE" == "darwin"* ]]; then
        sed -i '' \
            -e "s/MariaDB\( up to\| versions up to\| up to version\)\? [0-9]\+\.[0-9]\+/MariaDB up to $LATEST_MARIADB/g" \
            -e "s/MySQL\( up to\| versions up to\| up to version\)\? [0-9]\+\.[0-9]\+/MySQL up to $LATEST_MYSQL/g" \
            "$DOC_FILE" 2>/dev/null || true
    else
        sed -i \
            -e "s/MariaDB\( up to\| versions up to\| up to version\)\? [0-9]\+\.[0-9]\+/MariaDB up to $LATEST_MARIADB/g" \
            -e "s/MySQL\( up to\| versions up to\| up to version\)\? [0-9]\+\.[0-9]\+/MySQL up to $LATEST_MYSQL/g" \
            "$DOC_FILE" 2>/dev/null || true
    fi

    rm -f "$DOC_FILE.bak"
    set -e
    return 0
}

# Check for new versions
echo "Checking versions..."

if command -v curl >/dev/null 2>&1; then
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

    newer_mysql=$(echo "$mysql_versions" | awk -v latest="$LATEST_MYSQL" '
    function version_compare(v1, v2) {
        split(v1, a, ".")
        split(v2, b, ".")
        if (a[1] != b[1]) return a[1] - b[1]
        return a[2] - b[2]
    }
    version_compare($0, latest) > 0 { print $0 }')

    if [ -n "$newer_mariadb" ] || [ -n "$newer_mysql" ]; then
        echo "New versions detected! Update script."
        exit 1
    fi
fi

echo "OK"

# Versions to test
versions=(
    "mariadb:10.5" "mariadb:10.6" "mariadb:10.7" "mariadb:10.8"
    "mariadb:10.9" "mariadb:10.10" "mariadb:10.11"
    "mariadb:11.0" "mariadb:11.1" "mariadb:11.2" "mariadb:11.3-rc"
    "mariadb:11.4" "mariadb:11.5" "mariadb:11.6" "mariadb:11.7"
    "mariadb:11.8" "mariadb:12.0"
    "mariadb:latest"
    "mysql:5.6" "mysql:5.7"
    "mysql:8.0" "mysql:8.2" "mysql:8.3" "mysql:8.4"
    "mysql:9.0" "mysql:9.1" "mysql:9.2" "mysql:9.3" "mysql:9.4"
    "mysql:latest"
)

# Test versions
echo "Testing ${#versions[@]} versions..."

for version in "${versions[@]}"; do
    if [[ $version == mariadb* ]]; then
        db_type="mariadb"
        dump_cmd="mariadb-dump"
    else
        db_type="mysql"
        dump_cmd="mysqldump"
    fi

    # Pull and start container
    docker pull --platform linux/amd64 -q $version >/dev/null 2>&1
    docker run --rm -d --network=test_network --platform linux/amd64 --name db-test \
        -e MYSQL_ROOT_PASSWORD=pw $version bash -c "tail -f /dev/null" >/dev/null 2>&1
    sleep 5

    # Dump data
    docker exec db-test $dump_cmd -hmanticore -P9306 manticore t >dump.sql 2>/dev/null

    # Restore data
    docker exec manticore mysql -h0 -P9306 -e "DROP TABLE t;" >/dev/null 2>&1
    docker exec -i db-test $db_type -hmanticore -P9306 manticore <dump.sql >/dev/null 2>&1

    # Verify
    COUNT=$(docker exec db-test $db_type -hmanticore -P9306 -NBe "SELECT COUNT(*) FROM t;" manticore 2>/dev/null)

    if [ "$COUNT" = "20" ]; then
        echo "$version: OK"
    else
        echo "$version: FAIL"
        docker stop db-test >/dev/null 2>&1
        exit 1
    fi

    # Cleanup
    docker stop db-test >/dev/null 2>&1
    rm -f dump.sql
done

echo "Done"

# Update docs
update_documentation || true

echo "Success: MariaDB up to $LATEST_MARIADB, MySQL up to $LATEST_MYSQL"
exit 0
