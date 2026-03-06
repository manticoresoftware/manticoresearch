#!/bin/bash
set -euo pipefail

usage() {
	cat <<'EOF'
Run all ubertests in parallel.

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
work_root="${WORK_ROOT:-/tmp/utp_$timestamp}"

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
mkdir -p "$work_root"
rm -f "$log_dir"/failed.slot*

echo "Discovered ${#tests[@]} ubertests"
echo "Logs: $log_dir"

has_requirement() {
	local test_id="$1"
	local req="$2"
	local xml="$test_dir/test_${test_id}/test.xml"
	[ -f "$xml" ] && grep -Eiq "<${req}[[:space:]]*/>" "$xml"
}

select_tests_for_mode() {
	local selector="$1"
	selected_tests=()
	local id
	for id in "${tests[@]}"; do
		local non_rt=0
		local non_columnar=0
		local non_secondary=0
		has_requirement "$id" "non-rt" && non_rt=1
		has_requirement "$id" "non-columnar" && non_columnar=1
		has_requirement "$id" "non-secondary" && non_secondary=1

		case "$selector" in
			col_rt)
				if [ "$non_rt" -eq 0 ] && [ "$non_columnar" -eq 0 ]; then
					selected_tests+=("$id")
				fi
				;;
			secondary)
				if { [ "$non_rt" -eq 1 ] || [ "$non_columnar" -eq 1 ]; } && [ "$non_secondary" -eq 0 ]; then
					selected_tests+=("$id")
				fi
				;;
		esac
	done
}

run_worker() {
	local slot="$1"
	local mode_name="$2"
	local mode_args="$3"
	local db="test${slot}"
	local data_id="data${slot}"

	local idx id log rc
	for ((idx=slot-1; idx<${#selected_tests[@]}; idx+=workers)); do
		id="${selected_tests[$idx]}"
		log="$log_dir/test_${id}.${mode_name}.slot${slot}.log"
		work_dir="$work_root/${mode_name}/t${id}s${slot}"
		echo "[slot $slot][$mode_name] test_$id"
		mysql -uroot -e "DROP DATABASE IF EXISTS \`${db}\`; CREATE DATABASE \`${db}\`;"
		rm -rf "$work_dir"
		mkdir -p "$work_dir"
		cat > "$work_dir/guess.txt" <<'EOF'
global $g_guesscached; $g_guesscached = true;
global $g_re2; $g_re2 = true;
global $g_icu; $g_icu = true;
global $g_jieba; $g_jieba = true;
global $g_odbc; $g_odbc = true;
global $g_repli; $g_repli = true;
global $g_ssl; $g_ssl = true;
global $g_columnar_loaded; $g_columnar_loaded = true;
global $g_secondary_loaded; $g_secondary_loaded = true;
global $g_knn_loaded; $g_knn_loaded = true;
global $g_embeddings_loaded; $g_embeddings_loaded = true;
global $g_tzdata_loaded; $g_tzdata_loaded = true;
global $g_zlib; $g_zlib = true;
EOF
		if ! (
			cd "$test_dir"
			export CTEST_RESOURCE_GROUP_COUNT=1
			export CTEST_RESOURCE_GROUP_0_MYSQL="id:${db},slots:1"
			export CTEST_RESOURCE_GROUP_0_DATADIR="id:${data_id},slots:1"
			read -r -a _mode_args_array <<< "$mode_args"
			"$php_bin" ubertest.php \
				--ctest --strict-verbose --no-demo \
				-s /usr/bin/searchd -i /usr/bin/indexer \
				-u root -p '' \
				-t "${work_dir}/" \
				-tt "${work_dir}/" \
				"${_mode_args_array[@]}" \
				t "$id"
		) >"$log" 2>&1; then
			echo "test_$id FAILED ($mode_name, slot $slot), see $log"
			echo "${mode_name}:$id" >> "$log_dir/failed.slot$slot"
		elif grep -q "^SKIPPING " "$log"; then
			if grep -q "explicitly non-columnar test skipped in columnar mode" "$log"; then
				echo "test_$id SKIPPED (allowed non-columnar skip) ($mode_name, slot $slot)"
			else
				echo "test_$id SKIPPED (treated as failure) ($mode_name, slot $slot), see $log"
				echo "${mode_name}:$id" >> "$log_dir/failed.slot$slot"
			fi
		fi
	done
}

worker_failed=0
for mode_spec in \
	"columnar_rt|--rt --ignore-weights --columnar|col_rt" \
	"secondary||secondary"; do
	mode_name="${mode_spec%%|*}"
	rest="${mode_spec#*|}"
	mode="${rest%%|*}"
	mode_selector="${rest#*|}"
	select_tests_for_mode "$mode_selector"
	if [ "${#selected_tests[@]}" -eq 0 ]; then
		echo
		echo "Mode: $mode"
		echo "No tests selected for this mode."
		continue
	fi
	echo
	echo "Mode: $mode"
	echo "Running ${#selected_tests[@]} tests with $workers workers"

	pids=()
	for slot in $(seq 1 "$workers"); do
		run_worker "$slot" "$mode_name" "$mode" &
		pids+=("$!")
	done

	for pid in "${pids[@]}"; do
		if ! wait "$pid"; then
			worker_failed=1
		fi
	done
done

failed_ids="$log_dir/failed.ids"
cat "$log_dir"/failed.slot* 2>/dev/null | sort -u > "$failed_ids" || true

if [ -s "$failed_ids" ]; then
	echo
	echo "Failed tests (mode:test):"
	cat "$failed_ids"
	exit 1
fi

if [ "$worker_failed" -ne 0 ]; then
	echo
	echo "One or more worker processes exited unexpectedly."
	exit 1
fi

echo
echo "All tests passed."
