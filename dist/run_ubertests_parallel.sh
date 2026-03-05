#!/bin/bash
set -euo pipefail

usage() {
	cat <<'EOF'
Run all ubertests in parallel without ctest.

Usage:
  run_ubertests_parallel.sh [-j WORKERS]

Options:
  -j WORKERS   Number of parallel workers (default: detected CPU cores)
  -h           Show this help

Environment overrides:
  TEST_DIR     Ubertest directory (default: /test)
  PHP_BIN      PHP executable (default: php-real)
  LOG_DIR      Where per-test logs are written

Prerequisites (must be done once before running this script):
  1) MariaDB is running (e.g. /usr/local/bin/start-mysql)
  2) start-mysql has prepared databases test1..testN
EOF
}

detect_cpu_cores() {
	if command -v nproc >/dev/null 2>&1; then
		nproc
	elif command -v getconf >/dev/null 2>&1; then
		getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1
	else
		echo 1
	fi
}

workers="$(detect_cpu_cores)"
while getopts ":j:h" opt; do
	case "$opt" in
		j) workers="$OPTARG" ;;
		h)
			usage
			exit 0
			;;
		\?)
			echo "Unknown option: -$OPTARG" >&2
			usage
			exit 1
			;;
		:)
			echo "Missing argument for -$OPTARG" >&2
			usage
			exit 1
			;;
	esac
done

if ! [[ "$workers" =~ ^[1-9][0-9]*$ ]]; then
	echo "Invalid worker count: $workers" >&2
	exit 1
fi

test_dir="${TEST_DIR:-/test}"
php_bin="${PHP_BIN:-php-real}"
timestamp="$(date +%Y%m%d_%H%M%S)"
log_dir="${LOG_DIR:-$test_dir/ubertest-parallel-logs-$timestamp}"

if [ ! -d "$test_dir" ]; then
	echo "Test directory not found: $test_dir" >&2
	exit 1
fi

if ! command -v "$php_bin" >/dev/null 2>&1; then
	echo "PHP executable not found: $php_bin" >&2
	exit 1
fi

if ! command -v mysql >/dev/null 2>&1; then
	echo "mysql client is not installed in this container." >&2
	exit 1
fi

if ! mysql -uroot -e "SELECT 1" >/dev/null 2>&1; then
	echo "Cannot connect to MariaDB as root. Start DB first (e.g. /usr/local/bin/start-mysql)." >&2
	exit 1
fi

mapfile -t tests < <(
	find "$test_dir" -maxdepth 1 -type d -name 'test_[0-9]*' -printf '%f\n' \
	| sed 's/^test_//' \
	| sort -n
)

if [ "${#tests[@]}" -eq 0 ]; then
	echo "No ubertests found under $test_dir" >&2
	exit 1
fi

mkdir -p "$log_dir"
rm -f "$log_dir"/failed.slot*

echo "Running ${#tests[@]} ubertests with $workers workers"
echo "Logs: $log_dir"

run_worker() {
	local slot="$1"

	local idx id log rc
	for ((idx=slot-1; idx<${#tests[@]}; idx+=workers)); do
		id="${tests[$idx]}"
		log="$log_dir/test_${id}.slot${slot}.log"
		work_dir="$log_dir/work_test_${id}.slot${slot}"
		db="test$((idx+1))"
		data_id="data$((idx+1))"
		echo "[slot $slot] test_$id"
		mysql -uroot -e "DROP DATABASE IF EXISTS \`${db}\`; CREATE DATABASE \`${db}\`;"
		rm -rf "$work_dir"
		mkdir -p "$work_dir"
		if ! (
			cd "$test_dir"
			export CTEST_RESOURCE_GROUP_COUNT=1
			export CTEST_RESOURCE_GROUP_0_MYSQL="id:${db},slots:1"
			export CTEST_RESOURCE_GROUP_0_DATADIR="id:${data_id},slots:1"
			"$php_bin" ubertest.php \
				--ctest --strict-verbose --no-demo \
				-s /usr/bin/searchd -i /usr/bin/indexer \
				-u root -p '' \
				-tt "${work_dir}/" \
				t "$id"
		) >"$log" 2>&1; then
			echo "test_$id FAILED (slot $slot), see $log"
			echo "$id" >> "$log_dir/failed.slot$slot"
		fi
	done
}

pids=()
for slot in $(seq 1 "$workers"); do
	run_worker "$slot" &
	pids+=("$!")
done

worker_failed=0
for pid in "${pids[@]}"; do
	if ! wait "$pid"; then
		worker_failed=1
	fi
done

failed_ids="$log_dir/failed.ids"
cat "$log_dir"/failed.slot* 2>/dev/null | sort -n > "$failed_ids" || true

if [ -s "$failed_ids" ]; then
	echo
	echo "Failed tests:"
	sed 's/^/test_/' "$failed_ids"
	exit 1
fi

if [ "$worker_failed" -ne 0 ]; then
	echo
	echo "One or more worker processes exited unexpectedly."
	exit 1
fi

echo
echo "All tests passed."
