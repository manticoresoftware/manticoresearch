#!/usr/bin/env bash
# Isolate first-SHOW-SOURCES regression by swapping ONE Test Kit component at a time.
set -euo pipefail

CURRENT_IMAGE='ghcr.io/manticoresoftware/manticoresearch:test-kit-latest'
LEGACY_IMAGE='ghcr.io/manticoresoftware/manticoresearch:test-kit-b15e60f'
SUFFIX="${GITHUB_RUN_ID:-$$}"
NETWORK="image-components-${SUFFIX}"
KAFKA="image-components-kafka-${SUFFIX}"
CONTAINER=''
EXTRACT=''
WORK_DIR=$(mktemp -d)
RESULTS_DIR="${PWD}/executor-component-results"
mkdir -p "$RESULTS_DIR"

cleanup() {
  [[ -z "$CONTAINER" ]] || docker rm -f "$CONTAINER" >/dev/null 2>&1 || true
  [[ -z "$EXTRACT" ]] || docker rm -f "$EXTRACT" >/dev/null 2>&1 || true
  docker rm -f "$KAFKA" >/dev/null 2>&1 || true
  docker network rm "$NETWORK" >/dev/null 2>&1 || true
  rm -rf "$WORK_DIR"
}
trap cleanup EXIT

docker pull "$CURRENT_IMAGE"
docker pull "$LEGACY_IMAGE"
docker create --name "image-components-extract-${SUFFIX}" --entrypoint bash "$LEGACY_IMAGE" -lc 'sleep infinity' >/dev/null
EXTRACT="image-components-extract-${SUFFIX}"
docker cp "$EXTRACT:/usr/bin/manticore-executor" "$WORK_DIR/executor-84"
docker cp "$EXTRACT:/usr/bin/searchd" "$WORK_DIR/searchd-old"
docker cp "$EXTRACT:/usr/share/manticore/modules/manticore-buddy" "$WORK_DIR/buddy-old"
docker rm -f "$EXTRACT" >/dev/null
EXTRACT=''

sha256sum "$WORK_DIR/executor-84" "$WORK_DIR/searchd-old" > "$RESULTS_DIR/legacy-binaries.txt"
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
  -e CLUSTER_ID='MkU3OEVBNTcwNTJENDM2Qk' apache/kafka:4.1.0 >/dev/null
ready=0
for _ in $(seq 1 60); do
  if docker exec "$KAFKA" /opt/kafka/bin/kafka-broker-api-versions.sh --bootstrap-server localhost:9092 >/dev/null 2>&1; then ready=1; break; fi
  sleep 3
done
[[ "$ready" -eq 1 ]] || { docker logs "$KAFKA"; exit 1; }
docker exec "$KAFKA" /opt/kafka/bin/kafka-topics.sh --create --topic image_components --partitions 1 --bootstrap-server localhost:9092

cat > "$WORK_DIR/trace.php" <<'PHP'
<?php
declare(strict_types=1);
$root = '/usr/share/manticore/modules/manticore-buddy';
$patches = [
    [
        "$root/src/Lib/QueryProcessor.php",
        '$pluginPrefix = static::detectPluginPrefixFromRequest($request);',
        <<<'CODE'
$pluginPrefix = static::detectPluginPrefixFromRequest($request);
        file_put_contents('/tmp/qp-trace.log', "SELECTED $request->payload => $pluginPrefix\n", FILE_APPEND);
CODE
    ],
    [
        "$root/src/Lib/QueryProcessor.php",
        '$handler = new $handlerClassName($pluginPayload);',
        <<<'CODE'
$handler = new $handlerClassName($pluginPayload);
        file_put_contents('/tmp/qp-trace.log', "CONSTRUCTED $request->payload => $handlerClassName\n", FILE_APPEND);
CODE
    ],
    [
        "$root/src/Lib/QueryProcessor.php",
        '$hasMatch = $pluginPayloadClass::hasMatch($request);',
        <<<'CODE'
file_put_contents('/tmp/qp-trace.log', "CHECK $request->payload => {$plugin['short']}\n", FILE_APPEND);
                try {
                    $hasMatch = $pluginPayloadClass::hasMatch($request);
                    file_put_contents('/tmp/qp-trace.log', "CHECK_RESULT $request->payload => {$plugin['short']} = " . ($hasMatch ? 'true' : 'false') . "\n", FILE_APPEND);
                } catch (\Throwable $exception) {
                    file_put_contents('/tmp/qp-trace.log', "CHECK_EXCEPTION $request->payload => {$plugin['short']} = " . get_class($exception) . ': ' . $exception->getMessage() . "\n", FILE_APPEND);
                    throw $exception;
                }
CODE
    ],
    [
        "$root/src/Lib/QueryProcessor.php",
        '$pluginPayload = $payloadClassName::fromRequest($request);',
        <<<'CODE'
file_put_contents('/tmp/qp-trace.log', "FROM_REQUEST_START $request->payload => $pluginName\n", FILE_APPEND);
        $pluginPayload = $payloadClassName::fromRequest($request);
        file_put_contents('/tmp/qp-trace.log', "FROM_REQUEST_DONE $request->payload => $pluginName\n", FILE_APPEND);
CODE
    ],
    [
        "$root/src/Plugin/Queue/Handlers/BaseListHandler.php",
        '$tablePrefix = $this->getTablePrefix();',
        <<<'CODE'
$tablePrefix = $this->getTablePrefix();
        file_put_contents('/tmp/qp-trace.log', "LIST_HANDLER $tablePrefix\n", FILE_APPEND);
CODE
    ],
];
foreach ($patches as [$path, $before, $after]) {
    $code = file_get_contents($path);
    if (!is_string($code) || !str_contains($code, $before)) {
        fwrite(STDERR, "Missing instrumentation anchor in $path: $before\n");
        exit(1);
    }
    file_put_contents($path, str_replace($before, $after, $code));
}
PHP

printf 'variant\tfirst_show_sources\tsecond_show_sources\tstartup\n' > "$RESULTS_DIR/summary.tsv"
for variant in current old-php old-buddy old-searchd parser-fixed; do
  echo "======== $variant ========"
  mkdir -p "$RESULTS_DIR/$variant"
  CONTAINER="component-test-${SUFFIX}"
  docker run -d --platform linux/amd64 --network "$NETWORK" --name "$CONTAINER" \
    --entrypoint bash "$CURRENT_IMAGE" -lc 'sleep infinity' >/dev/null

  if [[ "$variant" == old-php ]]; then
    docker cp "$WORK_DIR/executor-84" "$CONTAINER:/usr/bin/manticore-executor"
  elif [[ "$variant" == old-buddy ]]; then
    docker cp "$WORK_DIR/buddy-old" "$CONTAINER:/tmp/buddy-old"
    docker exec "$CONTAINER" bash -ec '
      rm -rf /usr/share/manticore/modules/manticore-buddy
      mv /tmp/buddy-old /usr/share/manticore/modules/manticore-buddy
    '
  elif [[ "$variant" == old-searchd ]]; then
    docker cp "$WORK_DIR/searchd-old" "$CONTAINER:/usr/bin/searchd"
  elif [[ "$variant" == parser-fixed ]]; then
    docker exec "$CONTAINER" bash -ec '
      parser=/usr/share/manticore/modules/manticore-buddy/vendor/manticoresoftware/php-sql-parser/src/PHPSQLParser/processors/ShowProcessor.php
      grep -q "case .VIEWS.;" "$parser"
      sed -i "/case .VIEWS.;/s/;/:/" "$parser"
      grep -q "case .VIEWS.:$" "$parser"
    '
  fi

  docker exec "$CONTAINER" bash -lc '
    searchd --version
    manticore-executor -v
    sha256sum /usr/bin/searchd /usr/bin/manticore-executor
    git -C /usr/share/manticore/modules/manticore-buddy rev-parse HEAD
  ' > "$RESULTS_DIR/$variant/build-info.txt" 2>&1 || true
  cat "$RESULTS_DIR/$variant/build-info.txt"

  docker cp "$WORK_DIR/trace.php" "$CONTAINER:/tmp/trace.php"
  if ! docker exec "$CONTAINER" manticore-executor /tmp/trace.php > "$RESULTS_DIR/$variant/instrumentation.txt" 2>&1; then
    cat "$RESULTS_DIR/$variant/instrumentation.txt"
    echo -e "$variant\tSKIP\tSKIP\tINSTRUMENTATION_ERROR" >> "$RESULTS_DIR/summary.tsv"
  else
    docker exec "$CONTAINER" bash -lc 'stdbuf -oL searchd --logdebugvv >/dev/null 2>&1' \
      > "$RESULTS_DIR/$variant/startup.txt" 2>&1 || true
    ready=0
    for _ in $(seq 1 45); do
      if docker exec "$CONTAINER" mysql -h0 -P9306 -N -e 'SELECT 1' >/dev/null 2>&1 \
        && docker exec "$CONTAINER" bash -lc "grep -q '\\[BUDDY\\] started' /var/log/manticore/searchd.log" >/dev/null 2>&1; then
        ready=1
        break
      fi
      sleep 1
    done

    if [[ "$ready" -ne 1 ]]; then
      printf '%s\tSKIP\tSKIP\tSTARTUP_ERROR\n' "$variant" >> "$RESULTS_DIR/summary.tsv"
    else
      for sql in \
        "CREATE SOURCE absrc (id bigint, term text) type='kafka' broker_list='${KAFKA}:9092' topic_list='image_components' consumer_group='ic_${variant}' num_consumers='1' batch=50;" \
        'CREATE TABLE abdest (id bigint, term text);' \
        'CREATE MATERIALIZED VIEW abview TO abdest AS SELECT id, term FROM absrc;'; do
        docker exec "$CONTAINER" mysql -h0 -P9306 -e "$sql" \
          >> "$RESULTS_DIR/$variant/setup.txt" 2>&1 || true
      done

      first=ERROR
      second=ERROR
      if docker exec "$CONTAINER" mysql -h0 -P9306 -e 'SHOW SOURCES;' > "$RESULTS_DIR/$variant/first.txt" 2>&1; then first=OK; fi
      if docker exec "$CONTAINER" mysql -h0 -P9306 -e 'SHOW SOURCES;' > "$RESULTS_DIR/$variant/second.txt" 2>&1; then second=OK; fi
      printf '%s\t%s\t%s\tOK\n' "$variant" "$first" "$second" | tee -a "$RESULTS_DIR/summary.tsv"
      cat "$RESULTS_DIR/$variant/first.txt" "$RESULTS_DIR/$variant/second.txt"
    fi
  fi

  docker exec "$CONTAINER" bash -lc 'cat /tmp/qp-trace.log' > "$RESULTS_DIR/$variant/dispatch.txt" 2>&1 || true
  docker exec "$CONTAINER" bash -lc 'tail -n 400 /var/log/manticore/searchd.log' \
    > "$RESULTS_DIR/$variant/searchd.log" 2>&1 || true
  echo "Dispatcher trace for SHOW SOURCES:"
  grep -E 'SHOW SOURCES|CHECK_EXCEPTION|FROM_REQUEST|SELECTED|CONSTRUCTED|LIST_HANDLER' "$RESULTS_DIR/$variant/dispatch.txt" | tail -n 80 || true
  echo "Relevant searchd log:"
  grep -iE 'error|exception|fatal|buddy|swoole|failed' "$RESULTS_DIR/$variant/searchd.log" | tail -n 80 || true
  docker rm -f "$CONTAINER" >/dev/null
  CONTAINER=''
done
echo '=== COMPONENT RESULTS ==='
column -ts $'\t' "$RESULTS_DIR/summary.tsv" || cat "$RESULTS_DIR/summary.tsv"
