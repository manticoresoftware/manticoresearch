#!/bin/bash
set -e

# ============================================
# CONFIGURATION - Single Source of Truth
# ============================================
LATEST_KAFKA="4.1.0"

# Path to documentation
DOC_FILE="/manual/english/Integration/Kafka.md"

# Path to matrix test file (for version extraction)
MATRIX_TEST_FILE="test-integration-kafka-version.rec"

# ============================================
# FUNCTIONS (available when sourced)
# ============================================

# Get latest stable Kafka version from Docker Hub
get_latest_kafka() {
    docker run --rm alpine/curl sh -c '
    TAGS=$(curl -s "https://hub.docker.com/v2/repositories/apache/kafka/tags/?page_size=100" 2>/dev/null | \
           grep -oE "\"name\":\"[0-9]+\.[0-9]+\.[0-9]+\"" | \
           grep -oE "[0-9]+\.[0-9]+\.[0-9]+" | \
           grep -v -E "(rc|RC|alpha|beta|Alpha|Beta|ALPHA|BETA)" | \
           sort -V | uniq)
    echo "$TAGS" | tail -n 1
    ' 2>/dev/null
}

# Wait for Kafka to be ready
# Usage: wait_kafka_ready <container_name> [max_attempts] [sleep_time]
wait_kafka_ready() {
    local container=${1:-kafka}
    local max_attempts=${2:-60}
    local sleep_time=${3:-3}
    
    for i in $(seq 1 $max_attempts); do
        if docker exec $container /opt/kafka/bin/kafka-broker-api-versions.sh \
           --bootstrap-server localhost:9092 >/dev/null 2>&1; then
            return 0
        fi
        sleep $sleep_time
    done
    return 1
}

# Wait for Manticore to be ready
# Usage: wait_manticore_ready <container_name> [max_attempts] [sleep_time]
wait_manticore_ready() {
    local container=${1:-manticore}
    local max_attempts=${2:-40}
    local sleep_time=${3:-1}
    
    for i in $(seq 1 $max_attempts); do
        if docker exec $container mysql -h0 -P9306 -e "SELECT 1" >/dev/null 2>&1; then
            return 0
        fi
        sleep $sleep_time
    done
    return 1
}

# Create Kafka topic
# Usage: create_kafka_topic <container> <topic_name> <partitions>
create_kafka_topic() {
    local container=${1:-kafka}
    local topic=$2
    local partitions=${3:-1}
    
    docker exec $container /opt/kafka/bin/kafka-topics.sh \
        --create \
        --topic $topic \
        --partitions $partitions \
        --bootstrap-server localhost:9092 \
        2>&1 | grep -o "Created topic $topic\." | head -n 1
}

# Cleanup test environment
# Usage: cleanup_kafka_test
cleanup_kafka_test() {
    docker rm -f manticore kafka >/dev/null 2>&1 || true
    docker network rm app-network >/dev/null 2>&1 || true
}

# ============================================
# MAIN CHECKS (only when executed directly)
# ============================================

# If sourced, don't run checks - just provide functions and variables
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "🔍 Checking for new versions of Kafka..."
    echo ""

    # ============================================
    # CHECK 1: Docker Hub for new versions
    # ============================================
    if command -v curl >/dev/null 2>&1; then
        echo "1️⃣  Checking Docker Hub for new versions..."
        
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
            echo "   ⚠️  NEW Kafka versions detected:"
            echo "$newer_kafka" | sed 's/^/      - apache\/kafka:/'
            found_new=true
            echo ""
        fi

        if [ "$found_new" = false ]; then
            echo "   ✅ No new versions found after apache/kafka:$LATEST_KAFKA"
            echo ""
        else
            echo "   ❌ Action required:"
            echo "      1. Update LATEST_KAFKA variable in this script"
            echo "      2. Update version in documentation ($DOC_FILE)"
            echo "      3. Add new version(s) to matrix test ($MATRIX_TEST_FILE)"
            echo "      4. Run tests to verify compatibility"
            echo ""
            exit 1
        fi
    else
        echo "   ⚠️  curl not available - skipping Docker Hub check"
        echo ""
    fi

    # ============================================
    # CHECK 2: Documentation version
    # ============================================
    echo "2️⃣  Checking documentation versions..."

    if [ ! -f "$DOC_FILE" ]; then
        echo "   ⚠️  Documentation file not found at $DOC_FILE"
        echo ""
        echo "   Please ensure:"
        echo "      1. File manual/english/Integration/Kafka.md exists in repository"
        echo "      2. Directory is mounted: -v \$(pwd)/manual:/manual"
        echo ""
        exit 1
    fi

    # Extract version from documentation
    DOC_KAFKA=$(grep -o "apache/kafka:[0-9]\+\.[0-9]\+\.[0-9]\+" "$DOC_FILE" | grep -o "[0-9]\+\.[0-9]\+\.[0-9]\+" | head -1)

    if [ -z "$DOC_KAFKA" ]; then
        echo "   ❌ Could not find Kafka version in documentation"
        echo ""
        echo "   Please add to $DOC_FILE:"
        echo "   Currently, apache/kafka:$LATEST_KAFKA is tested and recommended."
        echo ""
        exit 1
    fi

    echo "   Script version: Kafka $LATEST_KAFKA"
    echo "   Documentation version: Kafka $DOC_KAFKA"

    # Check if they match
    if [ "$DOC_KAFKA" = "$LATEST_KAFKA" ]; then
        echo "   ✅ Documentation version matches script version"
        echo ""
    else
        echo "   ❌ Documentation version doesn't match script version!"
        echo ""
        echo "   Script has: Kafka $LATEST_KAFKA"
        echo "   Documentation has: Kafka $DOC_KAFKA"
        echo ""
        echo "   Please update documentation file:"
        echo "   manual/english/Integration/Kafka.md"
        echo ""
        echo "   Change the line to:"
        echo "   Currently, apache/kafka:$LATEST_KAFKA is tested and recommended."
        echo ""
        exit 1
    fi

    # ============================================
    # CHECK 3: Matrix test coverage
    # ============================================
    echo "3️⃣  Checking matrix test coverage..."

    if [ ! -f "$MATRIX_TEST_FILE" ]; then
        echo "   ⚠️  Matrix test file not found at $MATRIX_TEST_FILE"
        echo ""
    else
        # Extract versions from matrix test
        MATRIX_RAW=$(grep 'VERSIONS=(' "$MATRIX_TEST_FILE" 2>/dev/null | head -1)
        
        if [ -z "$MATRIX_RAW" ]; then
            echo "   ⚠️  Cannot parse VERSIONS from matrix test"
            echo ""
        else
            # Extract versions
            MATRIX_VERSIONS=$(echo "$MATRIX_RAW" | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')
            MATRIX_MAX=$(echo "$MATRIX_VERSIONS" | sort -V | tail -1)
            
            echo "   Matrix test covers up to: Kafka $MATRIX_MAX"
            echo "   Script LATEST_KAFKA: Kafka $LATEST_KAFKA"
            
            if [ "$MATRIX_MAX" = "$LATEST_KAFKA" ]; then
                echo "   ✅ Matrix test covers the latest version"
                echo ""
            else
                echo "   ❌ Matrix test doesn't cover the latest version!"
                echo ""
                echo "   Please update $MATRIX_TEST_FILE:"
                echo "   Add \"$LATEST_KAFKA\" to VERSIONS array"
                echo ""
                exit 1
            fi
        fi
    fi

    # ============================================
    # FINAL RESULT
    # ============================================
    echo "✅ All checks passed!"
    echo ""

    exit 0
fi
