#!/bin/bash
set -euo pipefail

mysql_data_dir=/var/lib/mysql
mysql_socket=/run/mysqld/mysqld.sock
mysql_init_sql=/run/mysqld/test-init.sql

mkdir -p /run/mysqld
chown -R mysql:mysql /run/mysqld "$mysql_data_dir"

if [ ! -d "$mysql_data_dir/mysql" ]; then
	if command -v mysqld >/dev/null 2>&1; then
		mysqld --initialize-insecure --user=mysql --datadir="$mysql_data_dir" >/dev/null
	else
		mysql_install_db --user=mysql --datadir="$mysql_data_dir" >/dev/null
	fi
fi

cat > "$mysql_init_sql" <<'SQL'
CREATE DATABASE IF NOT EXISTS test;
SQL
chown mysql:mysql "$mysql_init_sql"

mysqld \
	--user=mysql \
	--datadir="$mysql_data_dir" \
	--socket="$mysql_socket" \
	--bind-address=0.0.0.0 \
	--port=3306 \
	--skip-name-resolve \
	--init-file="$mysql_init_sql" \
	--log-error=/tmp/mysql.err &

for _ in $(seq 1 60); do
	if mysqladmin --socket="$mysql_socket" ping --silent >/dev/null 2>&1; then
		break
	fi
	sleep 1
done

mysqladmin --socket="$mysql_socket" ping --silent >/dev/null

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

mysql --socket="$mysql_socket" -uroot -e "CREATE DATABASE IF NOT EXISTS test"
for i in $(seq 1 "$core_count"); do
	mysql --socket="$mysql_socket" -uroot -e "CREATE DATABASE IF NOT EXISTS test${i}"
done

rm -f "$mysql_init_sql"

echo "MySQL is running on port 3306 with root user; prepared databases: test and test1..test${core_count}."
