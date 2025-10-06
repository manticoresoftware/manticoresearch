#!/bin/bash
set -e

# Check for new versions
echo "🔍 Checking for new versions of Kafka..."

LATEST_KAFKA="4.1.0"

if command -v curl >/dev/null 2>&1; then
    found_new=false

    # Fetch Kafka versions from Docker Hub, excluding RC/alpha/beta versions
    kafka_versions=$(curl -s "https://hub.docker.com/v2/repositories/apache/kafka/tags/?page_size=100" 2>/dev/null | \
        grep -o '"name":"[0-9]*\.[0-9]*\.[0-9]*"' | \
        grep -o '[0-9]*\.[0-9]*\.[0-9]*' | \
        grep -v -E '(rc|RC|alpha|beta|Alpha|Beta|ALPHA|BETA)' | \
        sort -u -V)

    newer_kafka=$(echo "$kafka_versions" | awk -v latest="$LATEST_KAFKA" '
    function version_compare(v1, v2) {
        split(v1, a, ".")
        split(v2, b, ".")
        if (a[1] != b[1]) return a[1] - b[1]
        if (a[2] != b[2]) return a[2] - b[2]
        return a[3] - b[3]
    }
    version_compare($0, latest) > 0 { print $0 }')

    if [ -n "$newer_kafka" ]; then
        echo "🆕 NEW Kafka versions detected:"
        echo "$newer_kafka" | sed 's/^/  - apache\/kafka:/'
        found_new=true
    fi

    if [ "$found_new" = false ]; then
        echo "✅ No new versions found after apache/kafka:$LATEST_KAFKA"
    else
        echo "⚠️ Please update the versions and test!"
        echo "⚠️ Also update:"
        echo "   - LATEST_KAFKA variable in this script"
        echo "   - Version number in documentation (manual/english/Integration/Kafka.md)"
        exit 1
    fi

    echo "✅ Version check completed"
else
    echo "⚠️ curl not available - skipping version check"
fi
echo ""

# Check documentation versions
echo "Checking documentation versions..."

DOC_FILE="/manual/english/Integration/Kafka.md"

if [ ! -f "$DOC_FILE" ]; then
    echo "❌ Documentation file not found at $DOC_FILE"
    echo ""
    echo "Please ensure:"
    echo "  1. File manual/english/Integration/Kafka.md exists in repository"
    echo "  2. Directory is mounted: -v \$(pwd)/manual:/manual"
    echo ""
    exit 1
fi

echo "Checking documentation file..."

# Extract version from documentation
DOC_KAFKA=$(grep -o "apache/kafka:[0-9]\+\.[0-9]\+\.[0-9]\+" "$DOC_FILE" | grep -o "[0-9]\+\.[0-9]\+\.[0-9]\+" | head -1)

if [ -z "$DOC_KAFKA" ]; then
    echo "❌ Could not find Kafka version in documentation"
    echo ""
    echo "Please add to $DOC_FILE:"
    echo "Currently, apache/kafka:$LATEST_KAFKA is tested and recommended."
    echo ""
    exit 1
fi

echo "Script version: Kafka $LATEST_KAFKA"
echo "Documentation version: Kafka $DOC_KAFKA"

# Check if they match
if [ "$DOC_KAFKA" = "$LATEST_KAFKA" ]; then
    echo "✅ Documentation version matches script version"
else
    echo "❌ Documentation version doesn't match script version!"
    echo ""
    echo "Script has: Kafka $LATEST_KAFKA"
    echo "Documentation has: Kafka $DOC_KAFKA"
    echo ""
    echo "Please update documentation file:"
    echo "manual/english/Integration/Kafka.md"
    echo ""
    echo "Change the line to:"
    echo "Currently, apache/kafka:$LATEST_KAFKA is tested and recommended."
    echo ""
    exit 1
fi

exit 0
