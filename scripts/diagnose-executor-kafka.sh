#!/usr/bin/env bash
# A/B diagnostic: identical Manticore Test Kit + Kafka; replace only the Executor binary.
set -euo pipefail

IMAGE_TAG='ghcr.io/manticoresoftware/manticoresearch:test-kit-latest'
RELEASE_DEB_URL='https://github.com/manticoresoftware/executor/releases/download/1.4.6/manticore-executor_1.4.6%2B26102314-a57785f2_amd64.deb'
DEBUG_TAR_URL='https://github.com/manticoresoftware/executor/releases/download/1.4.6/manticore-executor-1.4.6%2B26102314-a57785f2-linux-amd64-dev.tar.gz'
RESULTS_DIR="${RESULTS_DIR:-${PWD}/executor-ab-results}"
mkdir -p "$RESULTS_DIR"
RESULTS_DIR=$(cd "$RESULTS_DIR" && pwd)
WORK_DIR=$(mktemp -d)
SUFFIX="${GITHUB_RUN_ID:-$$}"
NETWORK="executor-ab-${SUFFIX}"
KAFKA="executor-ab-kafka-${SUFFIX}"
CURRENT_C=''

cleanup() {
  if [[ -n "$CURRENT_C" ]]; then docker rm -f "$CURRENT_C" >/dev/null 2>&1 || true; fi
  docker rm -f "$KAFKA" >/dev/null 2>&1 || true
  docker network rm "$NETWORK" >/dev/null 2>&1 || true
  rm -rf "$WORK_DIR"
}
trap cleanup EXIT

for cmd in docker curl; do
  command -v "$cmd" >/dev/null || { echo "Missing command: $cmd" >&2; exit 1; }
done

printf 'Downloading two reference Executor artifacts...\n'
curl -fLsS --retry 3 "$RELEASE_DEB_URL" -o "$WORK_DIR/release.deb"
curl -fLsS --retry 3 "$DEBUG_TAR_URL" -o "$WORK_DIR/debug.tar.gz"

docker pull "$IMAGE_TAG"
IMAGE_ID=$(docker image inspect --format '{{.Id}}' "$IMAGE_TAG")
printf 'Fixed Test Kit image: %s\n' "$IMAGE_ID" | tee "$RESULTS_DIR/image.txt"

docker network create "$NETWORK" >/dev/null
docker run -d --platform linux/amd64 --network "$NETWORK" --name "$KAFKA" \
  -e KAFKA_NODE_ID=1 \
  -e KAFKA_PROCESS_ROLES=broker,controller \
  -e "KAFKA_CONTROLLER_QUORUM_VOTERS=1@${KAFKA}:9093" \
  -e KAFKA_LISTENERS='PLAINTEXT://:9092,CONTROLLER://:9093' \
  -e "KAFKA_ADVERTISED_LISTENERS=PLAINTEXT://${KAFKA}:9092" \
  -e KAFKA_LISTENER_SECURITY_PROTOCOL_MAP='CONTROLLER:PLAINTEXT,PLAINTEXT:PLAINTEXT' \
  -e KAFKA_CONTROLLER_LISTENER_NAMES=CONTROLLER \
  -e KAFKA_OFFSETS_TOPIC_REPLICATION_FACTOR=1 \
  -e KAFKA_GROUP_INITIAL_REBALANCE_DELAY_MS=0 \
  -e CLUSTER_ID='MkU3OEVBNTcwNTJENDM2Qk' \
  apache/kafka:4.1.0 >/dev/null

printf 'Waiting for Kafka...\n'
KAFKA_READY=0
for _ in $(seq 1 60); do
  if docker exec "$KAFKA" /opt/kafka/bin/kafka-broker-api-versions.sh --bootstrap-server localhost:9092 >/dev/null 2>&1; then
    KAFKA_READY=1
    break
  fi
  sleep 3
done
if [[ "$KAFKA_READY" -ne 1 ]]; then
  docker logs "$KAFKA" | tail -n 150 >&2
  echo 'Kafka did not start' >&2
  exit 1
fi
docker exec "$KAFKA" /opt/kafka/bin/kafka-topics.sh --create \
  --topic executor_ab --partitions 1 --bootstrap-server localhost:9092

cat > "$WORK_DIR/queue-parse.php" <<'PHP'
<?php
declare(strict_types=1);
require '/usr/share/manticore/modules/manticore-buddy/vendor/autoload.php';
$parser = new \PHPSQLParser\PHPSQLParser();
foreach (['SHOW SOURCES;', 'SHOW SOURCES', 'SHOW MVS;', 'SHOW SOURCE absrc;', 'SHOW MV abview;'] as $sql) {
    echo "\nSQL: $sql\n";
    try {
        $parsed = $parser->parse($sql);
        echo 'PARSED: ' . json_encode($parsed, JSON_UNESCAPED_SLASHES) . "\n";
        $model = \Manticoresearch\Buddy\Base\Plugin\Queue\Models\SqlModelsHandler::handle($parsed);
        echo 'MODEL: ' . ($model ? get_class($model) : 'NONE') . "\n";
    } catch (\Throwable $e) {
        echo 'EXCEPTION: ' . get_class($e) . ': ' . $e->getMessage() . "\n";
    }
}
PHP

cat > "$WORK_DIR/trace-buddy.php" <<'PHP'
<?php
declare(strict_types=1);
$queueFile = '/usr/share/manticore/modules/manticore-buddy/src/Plugin/Queue/Payload.php';
$requestFile = '/usr/share/manticore/modules/manticore-buddy/vendor/manticoresoftware/buddy-core/src/Network/Request.php';
$patches = [
    [$queueFile,
     'public static function hasMatch(Request $request): bool {',
     <<<'CODE'
public static function hasMatch(Request $request): bool {
        file_put_contents('/tmp/buddy-queue-match.log', json_encode(['queue_payload'=>$request->payload], JSON_UNESCAPED_SLASHES) . "\n", FILE_APPEND);
CODE
    ],
    [$requestFile,
     <<<'CODE'
$this->command = strtolower(strtok($this->payload, ' ') ?: '');
CODE,
     <<<'CODE'
$this->command = strtolower(strtok($this->payload, ' ') ?: '');
        file_put_contents('/tmp/buddy-request-trace.log', json_encode([
            'raw' => $payload['message']['body'],
            'normalized' => $this->payload,
            'parser_error' => $payload['error']['message'],
        ], JSON_UNESCAPED_SLASHES) . "\n", FILE_APPEND);
CODE
    ],
];
foreach ($patches as [$path, $old, $new]) {
    $contents = file_get_contents($path);
    if ($contents === false || !str_contains($contents, $old)) {
        fwrite(STDERR, "Missing patch target in $path\n");
        exit(1);
    }
    file_put_contents($path, str_replace($old, $new, $contents));
    echo "Instrumented $path\n";
}
PHP

printf 'variant\tquery\tstatus\n' > "$RESULTS_DIR/summary.tsv"

run_query() {
  local variant="$1" container="$2" key="$3" sql="$4"
  local output="$RESULTS_DIR/$variant/$key.txt" status
  if docker exec "$container" mysql -h0 -P9306 -e "$sql" > "$output" 2>&1; then
    status=OK
  else
    status=ERROR
  fi
  printf '%s\t%s\t%s\n' "$variant" "$key" "$status" | tee -a "$RESULTS_DIR/summary.tsv"
  cat "$output"
}

for variant in previous-package current-package release-debug; do
  printf '\n========== %s ==========\n' "$variant"
  mkdir -p "$RESULTS_DIR/$variant"
  CURRENT_C="executor-ab-manticore-${SUFFIX}"
  docker run -d --platform linux/amd64 --network "$NETWORK" --name "$CURRENT_C" \
    --entrypoint bash "$IMAGE_ID" -lc 'sleep infinity' >/dev/null

  if [[ "$variant" == previous-package ]]; then
    docker cp "$WORK_DIR/release.deb" "$CURRENT_C:/tmp/release.deb"
    docker exec "$CURRENT_C" bash -ec '
      mkdir -p /tmp/old-executor
      dpkg-deb -x /tmp/release.deb /tmp/old-executor
      install -m 755 /tmp/old-executor/usr/bin/manticore-executor /usr/bin/manticore-executor
    '
  elif [[ "$variant" == release-debug ]]; then
    docker cp "$WORK_DIR/debug.tar.gz" "$CURRENT_C:/tmp/debug.tar.gz"
    docker exec "$CURRENT_C" bash -ec '
      mkdir -p /tmp/debug-executor
      tar -xzf /tmp/debug.tar.gz -C /tmp/debug-executor
      file=$(find /tmp/debug-executor -type f -name manticore-executor | head -n 1)
      test -n "$file"
      install -m 755 "$file" /usr/bin/manticore-executor
    '
  else
    # Check that the current Test Kit really contains the regression-suspect package.
    package_version=$(docker exec "$CURRENT_C" dpkg-query -W -f='${Version}' manticore-executor)
    echo "Current installed package: $package_version"
    if [[ "$package_version" != '1.4.6+26102310-445049b4-dev' ]]; then
      echo "Unexpected Test Kit version ($package_version); use an image pinned to the failing run." >&2
      exit 1
    fi
  fi

  docker exec "$CURRENT_C" bash -ec '
    set -x
    searchd --version
    manticore-executor -v
    sha256sum /usr/bin/searchd /usr/bin/manticore-executor
    git -C /usr/share/manticore/modules/manticore-buddy rev-parse HEAD
  ' > "$RESULTS_DIR/$variant/build-info.txt" 2>&1
  docker exec "$CURRENT_C" manticore-executor -m > "$RESULTS_DIR/$variant/modules.txt" 2>&1
  docker exec "$CURRENT_C" bash -lc 'sha256sum /usr/share/manticore/modules/manticore-buddy/composer.lock /usr/share/manticore/modules/manticore-buddy/vendor/composer/installed.php' >> "$RESULTS_DIR/$variant/build-info.txt" 2>&1 || true
  docker cp "$WORK_DIR/queue-parse.php" "$CURRENT_C:/tmp/queue-parse.php"
  docker exec "$CURRENT_C" manticore-executor /tmp/queue-parse.php \
    > "$RESULTS_DIR/$variant/parser.txt" 2>&1 || true
  cat "$RESULTS_DIR/$variant/build-info.txt"
  cat "$RESULTS_DIR/$variant/parser.txt"

  # Instrument Buddy inside this throwaway container, not in source control.
  docker cp "$WORK_DIR/trace-buddy.php" "$CURRENT_C:/tmp/trace-buddy.php"
  docker exec "$CURRENT_C" manticore-executor /tmp/trace-buddy.php

  docker exec "$CURRENT_C" bash -lc 'stdbuf -oL searchd --logdebugvv >/dev/null 2>&1'
  DB_READY=0
  for _ in $(seq 1 50); do
    if docker exec "$CURRENT_C" mysql -h0 -P9306 -N -e 'SELECT 1' >/dev/null 2>&1; then
      DB_READY=1
      break
    fi
    sleep 1
  done
  if [[ "$DB_READY" -ne 1 ]]; then
    echo "searchd did not start for $variant" >&2
    docker exec "$CURRENT_C" bash -lc 'tail -n 200 /var/log/manticore/searchd.log' || true
    exit 1
  fi

  # The same test fixture for each Executor. The Kafka topic exists, but we do not need to ingest data.
  for sql in \
    "CREATE SOURCE absrc (id bigint, term text) type='kafka' broker_list='${KAFKA}:9092' topic_list='executor_ab' consumer_group='ab_${variant}' num_consumers='1' batch=50;" \
    'CREATE TABLE abdest (id bigint, term text);' \
    'CREATE MATERIALIZED VIEW abview TO abdest AS SELECT id, term FROM absrc;'; do
    printf 'SETUP: %s\n' "$sql"
    docker exec "$CURRENT_C" mysql -h0 -P9306 -e "$sql"
  done

  run_query "$variant" "$CURRENT_C" show_sources 'SHOW SOURCES;'
  run_query "$variant" "$CURRENT_C" show_sources_no_semicolon 'SHOW SOURCES'
  run_query "$variant" "$CURRENT_C" show_mvs 'SHOW MVS;'
  run_query "$variant" "$CURRENT_C" show_source 'SHOW SOURCE absrc;'
  run_query "$variant" "$CURRENT_C" show_mv 'SHOW MV abview;'
  run_query "$variant" "$CURRENT_C" show_mv_vertical 'SHOW MV abview\G;'

  docker exec "$CURRENT_C" bash -lc 'tail -n 250 /var/log/manticore/searchd.log' \
    > "$RESULTS_DIR/$variant/searchd-tail.txt" 2>&1 || true
  docker exec "$CURRENT_C" bash -lc 'cat /tmp/buddy-request-trace.log /tmp/buddy-queue-match.log' \
    > "$RESULTS_DIR/$variant/buddy-trace.txt" 2>&1 || true
  echo "Buddy request/Queue matches ($variant):"
  cat "$RESULTS_DIR/$variant/buddy-trace.txt"
  docker rm -f "$CURRENT_C" >/dev/null
  CURRENT_C=''
done

# Compare with the Test Kit published for the last passing nightly (2026-10-23).
# This checks the WHOLE image, not just the Executor binary, including Composer vendors.
LEGACY_IMAGE='ghcr.io/manticoresoftware/manticoresearch:test-kit-b15e60f4'
printf '\n========== legacy-image (last green nightly commit) ==========\n'
if docker pull "$LEGACY_IMAGE"; then
  variant='legacy-image'
  mkdir -p "$RESULTS_DIR/$variant"
  CURRENT_C="executor-ab-manticore-${SUFFIX}"
  docker run -d --platform linux/amd64 --network "$NETWORK" --name "$CURRENT_C" \
    --entrypoint bash "$LEGACY_IMAGE" -lc 'sleep infinity' >/dev/null
  docker exec "$CURRENT_C" bash -lc '
    searchd --version
    manticore-executor -v
    sha256sum /usr/bin/searchd /usr/bin/manticore-executor
    git -C /usr/share/manticore/modules/manticore-buddy rev-parse HEAD
    sha256sum /usr/share/manticore/modules/manticore-buddy/composer.lock
    sha256sum /usr/share/manticore/modules/manticore-buddy/vendor/composer/installed.php
  ' > "$RESULTS_DIR/$variant/build-info.txt" 2>&1 || true
  cat "$RESULTS_DIR/$variant/build-info.txt"
  docker cp "$WORK_DIR/queue-parse.php" "$CURRENT_C:/tmp/queue-parse.php"
  docker exec "$CURRENT_C" manticore-executor /tmp/queue-parse.php \
    > "$RESULTS_DIR/$variant/parser.txt" 2>&1 || true
  cat "$RESULTS_DIR/$variant/parser.txt"
  docker cp "$WORK_DIR/trace-buddy.php" "$CURRENT_C:/tmp/trace-buddy.php"
  docker exec "$CURRENT_C" manticore-executor /tmp/trace-buddy.php
  docker exec "$CURRENT_C" bash -lc 'stdbuf -oL searchd --logdebugvv >/dev/null 2>&1'
  DB_READY=0
  for _ in $(seq 1 50); do
    if docker exec "$CURRENT_C" mysql -h0 -P9306 -N -e 'SELECT 1' >/dev/null 2>&1; then DB_READY=1; break; fi
    sleep 1
  done
  if [[ "$DB_READY" -ne 1 ]]; then
    echo 'legacy-image searchd failed to start' >&2
    docker exec "$CURRENT_C" bash -lc 'tail -n 200 /var/log/manticore/searchd.log' || true
  else
    for sql in \
      "CREATE SOURCE absrc (id bigint, term text) type='kafka' broker_list='${KAFKA}:9092' topic_list='executor_ab' consumer_group='ab_legacy' num_consumers='1' batch=50;" \
      'CREATE TABLE abdest (id bigint, term text);' \
      'CREATE MATERIALIZED VIEW abview TO abdest AS SELECT id, term FROM absrc;'; do
      docker exec "$CURRENT_C" mysql -h0 -P9306 -e "$sql"
    done
    run_query "$variant" "$CURRENT_C" show_sources 'SHOW SOURCES;'
    run_query "$variant" "$CURRENT_C" show_sources_no_semicolon 'SHOW SOURCES'
    run_query "$variant" "$CURRENT_C" show_mvs 'SHOW MVS;'
    run_query "$variant" "$CURRENT_C" show_source 'SHOW SOURCE absrc;'
    run_query "$variant" "$CURRENT_C" show_mv 'SHOW MV abview;'
    run_query "$variant" "$CURRENT_C" show_mv_vertical 'SHOW MV abview\G;'
  fi
  docker exec "$CURRENT_C" bash -lc 'tail -n 300 /var/log/manticore/searchd.log' \
    > "$RESULTS_DIR/$variant/searchd-tail.txt" 2>&1 || true
  docker exec "$CURRENT_C" bash -lc 'cat /tmp/buddy-request-trace.log /tmp/buddy-queue-match.log' \
    > "$RESULTS_DIR/$variant/buddy-trace.txt" 2>&1 || true
  echo "Buddy request/Queue matches ($variant):"
  cat "$RESULTS_DIR/$variant/buddy-trace.txt"
  docker rm -f "$CURRENT_C" >/dev/null
  CURRENT_C=''
else
  echo 'Legacy image tag not accessible: cannot compare the historic Test Kit.' | tee "$RESULTS_DIR/legacy-image-unavailable.txt"
fi

printf '\n=== FINAL RESULT ===\n'
column -ts $'\t' "$RESULTS_DIR/summary.tsv" 2>/dev/null || cat "$RESULTS_DIR/summary.tsv"
printf 'Full logs: %s\n' "$RESULTS_DIR"
