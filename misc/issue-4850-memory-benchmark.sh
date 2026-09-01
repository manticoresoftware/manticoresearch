#!/usr/bin/env bash
set -euo pipefail

# Peak-RSS benchmark for GitHub issue #4850. Runs an isolated local searchd,
# creates several RT disk chunks with high-cardinality terms, then merges them.

searchd=${SEARCHD:-./build-4850/src/searchd}
chunks=${CHUNKS:-10}
docs_per_chunk=${DOCS_PER_CHUNK:-1000}
words_per_doc=${WORDS_PER_DOC:-8}
word_suffix_len=${WORD_SUFFIX_LEN:-24}
min_infix_len=${MIN_INFIX_LEN:-3}
port=${PORT:-$((19000 + $$ % 1000))}
root=${RUN_DIR:-$(pwd)/issue-4850-run-$min_infix_len-$$}

sql() {
    mysql --protocol=tcp -h127.0.0.1 -P"$port" --batch --raw --skip-column-names "$@"
}

cleanup() {
    if [ -s "$root/searchd.pid" ]; then
        "$searchd" --config "$root/manticore.conf" --stopwait >/dev/null 2>&1 || true
    fi
    [ "${KEEP_RUN_DIR:-0}" = 1 ] || rm -rf "$root"
}
trap cleanup EXIT

mkdir -p "$root/data" "$root/binlog"
cat >"$root/manticore.conf" <<EOF
searchd {
    listen = 127.0.0.1:$port:mysql
    log = $root/searchd.log
    query_log = $root/query.log
    pid_file = $root/searchd.pid
    data_dir = $root/data
    binlog_path = $root/binlog
}
EOF

"$searchd" --config "$root/manticore.conf" >"$root/stdout.log" 2>"$root/stderr.log"
for _ in $(seq 1 60); do
    printf 'SELECT 1;\n' | sql >/dev/null 2>&1 && break
    sleep 1
done
printf 'SELECT 1;\n' | sql >/dev/null

pid=$(cat "$root/searchd.pid")
: >"$root/rss.tsv"
(
    while kill -0 "$pid" 2>/dev/null; do
        ps -o rss=,vsz= -p "$pid" | awk -v now="$(date +%s)" '{print now, $1, $2}' >>"$root/rss.tsv"
        sleep 0.1
    done
) &
sampler=$!

printf "CREATE TABLE t(title text) min_infix_len='%s' morphology='none';\n" "$min_infix_len" | sql
printf 'SET GLOBAL parallel_chunk_merges = 1;\n' | sql

for chunk in $(seq 1 "$chunks"); do
    start=$((1 + (chunk - 1) * docs_per_chunk))
    awk -v start="$start" -v docs="$docs_per_chunk" -v words="$words_per_doc" -v suffix_len="$word_suffix_len" 'BEGIN {
        alphabet = "abcdefghijklmnopqrstuvwxyz0123456789"
        q = sprintf("%c", 39)
        printf "INSERT INTO t(id,title) VALUES "
        for (doc = start; doc < start + docs; doc++) {
            srand(doc)
            title = ""
            for (w = 0; w < words; w++) {
                suffix = ""
                for (i = 0; i < suffix_len; i++)
                    suffix = suffix substr(alphabet, int(rand() * length(alphabet)) + 1, 1)
                title = title (w ? " " : "") sprintf("w%08x%x%s", doc, w, suffix)
            }
            printf "%s(%d,%s%s%s)", (doc == start ? "" : ","), doc, q, title, q
        }
        printf ";\n"
    }' | sql
    printf 'FLUSH RAMCHUNK t;\n' | sql >/dev/null
done

printf 'SHOW TABLE t STATUS LIKE '\''disk_chunks'\'';\n' | sql
echo START_OPTIMIZE
started=$SECONDS
printf 'OPTIMIZE TABLE t OPTION cutoff=1;\n' | sql >"$root/optimize.out" 2>"$root/optimize.err" &
optimizer=$!

while kill -0 "$pid" 2>/dev/null && [ $((SECONDS - started)) -le 900 ]; do
    printf 'SHOW TABLE t STATUS LIKE '\''disk_chunks'\'';\nSHOW TABLE t STATUS LIKE '\''optimizing'\'';\n' | sql >"$root/status.out" 2>&1 || true
    if awk '$1=="disk_chunks" && $2=="1"{chunks=1} $1=="optimizing" && $2=="0"{idle=1} END{exit(chunks && idle ? 0 : 1)}' "$root/status.out"; then
        break
    fi
    sleep 1
done
wait "$optimizer" || true
kill "$sampler" >/dev/null 2>&1 || true
wait "$sampler" 2>/dev/null || true

printf 'SELECT id FROM t WHERE MATCH('\''*00000001*'\'') LIMIT 5;\n' | sql >"$root/query.out"
awk 'BEGIN{rss=0;vsz=0} $2>rss{rss=$2} $3>vsz{vsz=$3} END{print "peak_rss_kb=" rss; print "peak_vsz_kb=" vsz}' "$root/rss.tsv"
echo "merge_seconds=$((SECONDS - started))"
echo "query_rows=$(awk 'END{print NR+0}' "$root/query.out")"
find "$root/data" -type f -name '*.spi' -exec stat -f 'spi_bytes=%z' {} \; -exec shasum -a 256 {} \;
echo "run_dir=$root"
