#!/bin/bash
set -euo pipefail

mysql_data_dir=/var/lib/mysql
mysql_socket=/run/mysqld/mysqld.sock
mysql_init_sql=/run/mysqld/test-init.sql

mysqld_bin="$(command -v mariadbd || true)"
mysql_bin="$(command -v mysql || true)"

if [ -z "$mysqld_bin" ] || [ -z "$mysql_bin" ]; then
	echo "Required binaries are missing: mariadbd='$mysqld_bin' mysql='$mysql_bin'" >&2
	exit 1
fi

mkdir -p /run/mysqld
chown -R mysql:mysql /run/mysqld "$mysql_data_dir"

if [ ! -d "$mysql_data_dir/mysql" ]; then
	if command -v mariadb-install-db >/dev/null 2>&1; then
		mariadb-install-db --user=mysql --datadir="$mysql_data_dir" >/dev/null
	else
		echo "mariadb-install-db is required but not found" >&2
		exit 1
	fi
fi

cat > "$mysql_init_sql" <<'SQL'
CREATE DATABASE IF NOT EXISTS test;
SQL
chown mysql:mysql "$mysql_init_sql"

"$mysqld_bin" \
	--user=mysql \
	--datadir="$mysql_data_dir" \
	--socket="$mysql_socket" \
	--bind-address=0.0.0.0 \
	--port=3306 \
	--skip-name-resolve \
	--init-file="$mysql_init_sql" \
	--log-error=/tmp/mysql.err &

for _ in $(seq 1 60); do
	if "$mysql_bin" --socket="$mysql_socket" -uroot -e "SELECT 1" >/dev/null 2>&1; then
		break
	fi
	sleep 1
done

"$mysql_bin" --socket="$mysql_socket" -uroot -e "SELECT 1" >/dev/null

if command -v nproc >/dev/null 2>&1; then
	core_count="$(nproc)"
elif command -v getconf >/dev/null 2>&1; then
	core_count="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
else
	core_count=1
fi

if ! [[ "$core_count" =~ ^[1-9][0-9]*$ ]]; then
	core_count=1
fi

"$mysql_bin" --socket="$mysql_socket" -uroot -e "CREATE DATABASE IF NOT EXISTS test"
for i in $(seq 1 "$core_count"); do
	"$mysql_bin" --socket="$mysql_socket" -uroot -e "CREATE DATABASE IF NOT EXISTS test${i}"
done

rm -f "$mysql_init_sql"

echo "MySQL is running on port 3306 with root user; prepared databases: test and test1..test${core_count}."
