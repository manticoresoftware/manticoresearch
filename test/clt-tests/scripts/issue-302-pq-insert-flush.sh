#!/usr/bin/env bash
# Standalone PQ INSERT/FLUSH stress regression (bash, curl, python3 required).
# MRE_SEARCHD=/path/to/build/src/searchd MRE_PORT=<free-port> bash this-script
# Defaults: 3000 rules, single-row inserts, 20ms delay, flush every 200ms.
# MRE_KEEP=1 retains evidence; success requires count=3000 and no flush errors.
set -Eeuo pipefail

SEARCHD=${MRE_SEARCHD:-/usr/bin/searchd}
BASE=${MRE_LOCAL_BASE:-/tmp/issue302-local-$$}
PORT=${MRE_PORT:?Set MRE_PORT to a free nonreserved HTTP port; the next port is used for MySQL}
SQL="http://127.0.0.1:${PORT}/sql"
TOTAL=${MRE_TOTAL:-3000}
BATCH=${MRE_BATCH:-1}
DELAY=${MRE_DELAY:-0.02}
REQUEST_TIMEOUT=${MRE_REQUEST_TIMEOUT:-10}
FLUSH_INTERVAL=${MRE_FLUSH_INTERVAL:-0.2}
[[ ! -e "$BASE" ]] || { printf "Refusing existing runtime directory: %s\n" "$BASE" >&2; exit 1; }
mkdir -p "$BASE/data" "$BASE/log"
CONFIG="$BASE/manticore.conf"
cat >"$CONFIG" <<EOF
searchd {
    listen = 127.0.0.1:$PORT:http
    listen = 127.0.0.1:$((PORT+1)):mysql41
    data_dir = $BASE/data
    pid_file = $BASE/searchd.pid
    log = $BASE/log/searchd.log
    query_log = $BASE/log/query.log
    query_log_format = sphinxql
    seamless_rotate = 1
}
EOF
cleanup() {
    [[ -n "${FLUSH_PID:-}" ]] && kill "$FLUSH_PID" >/dev/null 2>&1 || true
    if [[ -n "${SEARCHD_PID:-}" ]] && kill -0 "$SEARCHD_PID" >/dev/null 2>&1; then
        kill "$SEARCHD_PID" >/dev/null 2>&1 || true
        for _ in 1 2 3 4 5; do
            kill -0 "$SEARCHD_PID" >/dev/null 2>&1 || break
            sleep 1
        done
        kill -9 "$SEARCHD_PID" >/dev/null 2>&1 || true
    fi
    if [[ ${MRE_KEEP:-0} == 1 ]]; then printf 'Evidence retained: %s\n' "$BASE"; else rm -rf "$BASE"; fi
}
trap cleanup EXIT
"$SEARCHD" --config "$CONFIG" --nodetach >"$BASE/searchd.stdout" 2>&1 &
SEARCHD_PID=$!
for _ in $(seq 1 60); do
    curl -fsS --max-time 2 --data-urlencode mode=raw --data-urlencode 'query=SHOW TABLES' "$SQL" >/dev/null 2>&1 && break
    kill -0 "$SEARCHD_PID" >/dev/null 2>&1 || { cat "$BASE/searchd.stdout" >&2; exit 1; }
    sleep 1
done
curl -fsS --max-time 5 --data-urlencode mode=raw --data-urlencode 'query=SHOW TABLES' "$SQL" >/dev/null
printf 'version: '; "$SEARCHD" --version | sed -n '1p'
printf 'settings: total=%s batch=%s delay=%ss flush=%ss port=%s\n' "$TOTAL" "$BATCH" "$DELAY" "$FLUSH_INTERVAL" "$PORT"
DDL="CREATE TABLE t1 (id bigint, query text indexed, tags text indexed, filters text indexed, \`json\` json) type='pq'"
DDL_RESPONSE=$(curl -sS --max-time 30 --data-urlencode mode=raw --data-urlencode "query=$DDL" "$SQL")
[[ "$DDL_RESPONSE" == *'"error":""'* ]] || { printf '%s\n' "$DDL_RESPONSE" >&2; exit 1; }
flush_loop() {
    while [[ ! -e "$BASE/stop-flush" ]]; do
        local response
        if response=$(curl -sS --max-time "$REQUEST_TIMEOUT" --data-urlencode mode=raw --data-urlencode 'query=FLUSH RAMCHUNK t1' "$SQL" 2>>"$BASE/flush-errors.log"); then
            [[ "$response" == *'"error":""'* ]] || printf 'SQL: %s\n' "$response" >>"$BASE/flush-errors.log"
        else
            printf 'curl rc=%d\n' "$?" >>"$BASE/flush-errors.log"
        fi
        sleep "$FLUSH_INTERVAL"
    done
}
flush_loop & FLUSH_PID=$!
sql_lit() { local v=$1; v=${v//\\/\\\\}; v=${v//\'/\\\'}; printf "'%s'" "$v"; }
QUERY='((alpha NEAR/10 beta) AND (gamma OR delta) AND (epsilon | zeta)) '; for _ in $(seq 1 160); do QUERY+='alpha beta gamma delta epsilon zeta '; done
start=$(date +%s)
insert_batch() {
    local first=$1 last=$2 sql='INSERT INTO t1 (id, query, tags, filters) VALUES ' i row q t f response
    for ((i=first;i<=last;i++)); do
        q="$QUERY rule_$i customer_$((i%997))"
        t="{\"tag\":\"production_lift_$((i%251))_$i\",\"source\":\"issue302-mre\",\"generation\":\"$i\"}"
        f="(json.language='en' OR json.language='de') AND (json.provider=$((i%521)))"
        row="($i,$(sql_lit "$q"),$(sql_lit "$t"),$(sql_lit "$f"))"; ((i<last)) && row+=','; sql+="$row"
    done
    response=$(printf '%s' "$sql" | curl -sS --max-time "$REQUEST_TIMEOUT" --data-urlencode mode=raw --data-urlencode query@- "$SQL") || return $?
    [[ "$response" == *'"error":""'* ]] || { printf 'insert SQL error: %.500s\n' "$response" >&2; return 65; }
}
for ((first=1,b=0;first<=TOTAL;first+=BATCH,b++)); do
    last=$((first+BATCH-1)); ((last>TOTAL)) && last=$TOTAL
    if insert_batch "$first" "$last"; then :; else
        rc=$?
        printf 'INSERT FAILED rc=%d at batch %d (rules %d..%d); curl 28 means timeout, 65 means SQL error\n' "$rc" "$b" "$first" "$last"
        if [[ -n ${MRE_DIAG:-} ]]; then "$MRE_DIAG" "$SEARCHD_PID" "$BASE" || true; fi
        exit "$rc"
    fi
    now=$(date +%s); (( BATCH>1 || b%100==0 || last==TOTAL )) && printf 'batch %d: %d/%d rules, %ds\n' "$b" "$last" "$TOTAL" "$((now-start))"
    sleep "$DELAY"
done
touch "$BASE/stop-flush"
wait "$FLUSH_PID"
FLUSH_PID=
[[ ! -s "$BASE/flush-errors.log" ]] || { printf 'FLUSH failed; see %s/flush-errors.log\n' "$BASE"; exit 1; }
curl -fsS --max-time "$REQUEST_TIMEOUT" --data-urlencode mode=raw --data-urlencode 'query=SELECT COUNT(*) AS n FROM t1' "$SQL" >"$BASE/count.json"
python3 - "$BASE/count.json" "$TOTAL" <<'PY'
import json,sys
r=json.load(open(sys.argv[1]))[0]
assert not r['error'],r
n=r['data'][0]['n']
assert int(n)==int(sys.argv[2]),r
print('verified count:',n)
PY
echo 'loaded without hang'
curl -fsS --max-time 5 --data-urlencode mode=raw --data-urlencode 'query=SHOW THREADS' "$SQL" >/dev/null && echo 'SHOW THREADS: responsive'
grep 'saved ok' "$BASE/log/searchd.log" | tail -n 5 || true
