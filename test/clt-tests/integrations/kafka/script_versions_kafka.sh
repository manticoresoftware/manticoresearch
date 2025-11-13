#!/bin/bash
set -e

# ============================================
# CONFIGURATION
# ============================================
LATEST_KAFKA="4.1.0"
MIN_KAFKA="3.7.0"
DOC_FILE="/manual/english/Integration/Kafka.md"
MATRIX_TEST_FILE="./test/clt-tests/integrations/kafka/test-integration-kafka-version.rec"

# ============================================
# FUNCTIONS
# ============================================

version_compare() {
    local v1=$1
    local v2=$2
    IFS='.' read -ra V1 <<< "$v1"
    IFS='.' read -ra V2 <<< "$v2"
    for i in 0 1 2; do
        local num1=${V1[$i]:-0}
        local num2=${V2[$i]:-0}
        if [ "$num1" -gt "$num2" ]; then return 1
        elif [ "$num1" -lt "$num2" ]; then return 2
        fi
    done
    return 0
}

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

create_kafka_topic() {
    local container=${1:-kafka}
    local topic=$2
    local partitions=${3:-1}
    docker exec $container /opt/kafka/bin/kafka-topics.sh \
        --create --topic $topic --partitions $partitions \
        --bootstrap-server localhost:9092 >/dev/null 2>&1
}

cleanup_kafka_test() {
    docker rm -f manticore kafka >/dev/null 2>&1 || true
    docker network rm app-network >/dev/null 2>&1 || true
}

# ============================================
# MAIN CHECKS
# ============================================

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "🔍 Kafka Version Check"
    echo ""

    # CHECK 1: Docker Hub
    echo "1️⃣  Docker Hub..."
    if command -v curl >/dev/null 2>&1 && command -v docker >/dev/null 2>&1; then
        kafka_versions=$(curl -s "https://hub.docker.com/v2/repositories/apache/kafka/tags/?page_size=100" 2>/dev/null | \
            grep -o '"name":"[0-9]*\.[0-9]*\.[0-9]*"' | grep -o '[0-9]*\.[0-9]*\.[0-9]*' | \
            grep -v -E '(rc|RC|alpha|beta|Alpha|Beta|ALPHA|BETA)' | sort -u -V)

        newer_kafka=$(echo "$kafka_versions" | awk -v latest="$LATEST_KAFKA" '
        function version_compare(v1, v2) {
            split(v1, a, "."); split(v2, b, ".")
            if (a[1] != b[1]) return a[1] - b[1]
            if (a[2] != b[2]) return a[2] - b[2]
            return a[3] - b[3]
        }
        version_compare($0, latest) > 0 { print $0 }')

        if [ -n "$newer_kafka" ]; then
            echo "   ⚠️  New versions:"
            echo "$newer_kafka" | sed 's/^/      - /'
            echo ""
            echo "   Update: MIN_KAFKA, LATEST_KAFKA, Kafka.md, VERSIONS[]"
            exit 1
        fi
        echo "   ✅ No new versions after $LATEST_KAFKA"
    else
        echo "   ⚠️  curl/docker not available"
    fi
    echo ""

    # CHECK 2: Documentation
    echo "2️⃣  Documentation..."
    if [ ! -f "$DOC_FILE" ]; then
        echo "   ⚠️  File not found: $DOC_FILE"
        exit 1
    fi

    DOC_RANGE=$(grep -oE "[0-9]+\.[0-9]+\.[0-9]+-[0-9]+\.[0-9]+\.[0-9]+" "$DOC_FILE" | head -1)
    if [ -z "$DOC_RANGE" ]; then
        echo "   ❌ Version range not found in docs"
        echo "   Expected: versions $MIN_KAFKA-$LATEST_KAFKA"
        exit 1
    fi

    DOC_MIN=$(echo "$DOC_RANGE" | cut -d'-' -f1)
    DOC_MAX=$(echo "$DOC_RANGE" | cut -d'-' -f2)

    if [ "$DOC_MIN" = "$MIN_KAFKA" ] && [ "$DOC_MAX" = "$LATEST_KAFKA" ]; then
        echo "   ✅ Range: $DOC_MIN - $DOC_MAX"
    else
        echo "   ❌ Mismatch:"
        echo "      Script: $MIN_KAFKA - $LATEST_KAFKA"
        echo "      Docs:   $DOC_MIN - $DOC_MAX"
        exit 1
    fi
    echo ""

    # CHECK 3: Matrix test
    echo "3️⃣  Matrix test..."
    if [ ! -f "$MATRIX_TEST_FILE" ]; then
        echo "   ⚠️  File not found: $MATRIX_TEST_FILE"
    else
        MATRIX_RAW=$(grep 'VERSIONS=(' "$MATRIX_TEST_FILE" 2>/dev/null | head -1)
        if [ -z "$MATRIX_RAW" ]; then
            echo "   ⚠️  Cannot parse VERSIONS"
        else
            MATRIX_VERSIONS=$(echo "$MATRIX_RAW" | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | sort -V)
            MATRIX_MIN=$(echo "$MATRIX_VERSIONS" | head -1)
            MATRIX_MAX=$(echo "$MATRIX_VERSIONS" | tail -1)
            MATRIX_COUNT=$(echo "$MATRIX_VERSIONS" | wc -l)

            if [ "$MATRIX_MIN" = "$MIN_KAFKA" ] && [ "$MATRIX_MAX" = "$LATEST_KAFKA" ]; then
                echo "   ✅ Coverage: $MATRIX_MIN - $MATRIX_MAX ($MATRIX_COUNT versions)"
            else
                echo "   ❌ Coverage mismatch:"
                echo "      Script: $MIN_KAFKA - $LATEST_KAFKA"
                echo "      Matrix: $MATRIX_MIN - $MATRIX_MAX"
                exit 1
            fi
        fi
    fi
    echo ""

    echo "✅ All checks passed"
    echo "   Range: $MIN_KAFKA - $LATEST_KAFKA"
    exit 0
fi
