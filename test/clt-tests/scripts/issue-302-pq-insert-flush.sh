#!/usr/bin/env bash
# Standalone PQ INSERT / FLUSH RAMCHUNK stress test. Requires curl and python3.
# Example: MRE_PORT=19588 MRE_SEARCHD=/path/to/searchd bash this-file
set -euo pipefail

SEARCHD=${MRE_SEARCHD:-searchd}
PORT=${MRE_PORT:?Set a free HTTP port (the next port is used for MySQL)}
TOTAL=${MRE_TOTAL:-3000}
TIMEOUT=${MRE_REQUEST_TIMEOUT:-10}
SQL="http://127.0.0.1:$PORT/sql"
BASE=$(mktemp -d)
searchd_pid=''
flush_pid=''
cleanup() {
    for pid in "$flush_pid" "$searchd_pid"; do
        if [[ -n "$pid" ]]; then kill "$pid" 2>/dev/null || true; fi
    done
    if [[ ${MRE_KEEP:-0} == 1 ]]; then
        echo "Logs retained: $BASE"
    else
        rm -rf "$BASE"
    fi
}
trap cleanup EXIT
mkdir -p "$BASE/data"
cat >"$BASE/manticore.conf" <<CONF
searchd {
    listen = 127.0.0.1:$PORT:http
    listen = 127.0.0.1:$((PORT+1)):mysql41
    data_dir = $BASE/data
    binlog_path = $BASE/data
    pid_file = $BASE/searchd.pid
    log = $BASE/searchd.log
}
CONF
sql() {
    local reply
    reply=$(curl -fsS --max-time "$TIMEOUT" \
        --data-urlencode mode=raw --data-urlencode "query=$1" "$SQL") || return 1
    [[ "$reply" == *'"error":""'* ]] || { echo "$reply" >&2; return 1; }
}
"$SEARCHD" --config "$BASE/manticore.conf" --nodetach >"$BASE/searchd.stdout" 2>&1 &
searchd_pid=$!
ready=0
for _ in {1..60}; do
    if sql 'SHOW TABLES' >/dev/null 2>&1; then ready=1; break; fi
    kill -0 "$searchd_pid" 2>/dev/null || { cat "$BASE/searchd.stdout" >&2; exit 1; }
    sleep 1
done
[[ $ready == 1 ]] || { echo 'searchd did not start' >&2; exit 1; }
sql "CREATE TABLE pq_flush_race (body text, meta json) type='pq'"
query=$(printf 'alpha beta gamma delta epsilon zeta %.0s' {1..160})
(
    while [[ ! -e "$BASE/stop" ]]; do
        sql 'FLUSH RAMCHUNK pq_flush_race' || exit 1
        echo . >>"$BASE/flush.count"
        sleep 0.2
    done
) >"$BASE/flush.log" 2>&1 &
flush_pid=$!
for ((i=1; i<=TOTAL; i++)); do
    sql "INSERT INTO pq_flush_race(id,query) VALUES ($i,'${query}rule_$i')" || {
        echo "INSERT failed at rule $i" >&2
        exit 1
    }
    sleep 0.02
done
touch "$BASE/stop"
wait "$flush_pid" || { cat "$BASE/flush.log" >&2; exit 1; }
flush_pid=''
[[ -s "$BASE/flush.count" ]] || { echo 'No successful FLUSH' >&2; exit 1; }
response=$(curl -fsS --max-time "$TIMEOUT" --data-urlencode mode=raw \
    --data-urlencode 'query=SELECT COUNT(*) AS n FROM pq_flush_race' "$SQL")
python3 -c 'import json,sys; r=json.load(sys.stdin)[0]; assert not r["error"],r; assert int(r["data"][0]["n"])==int(sys.argv[1]),r' "$TOTAL" <<<"$response"
echo "PQ INSERT/FLUSH: ok ($TOTAL rules)"
