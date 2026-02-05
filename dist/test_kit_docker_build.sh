#!/bin/bash
set -e

# Ensure a clean clone directory so reruns don't fail.
rm -rf docker
# Shallow clone with retries to avoid transient network errors.
for i in 1 2 3; do
	if git clone --depth 1 https://github.com/manticoresoftware/docker.git docker; then
		break
	fi
	echo "git clone failed (attempt $i), retrying..."
	rm -rf docker
	sleep 2
done
if [ ! -d docker ]; then
	echo "ERROR: failed to clone docker repo after retries" >&2
	exit 1
fi
cd docker

# We set it later when parse deps.txt
executor_dev_path=

# Downloads a package from a repository by given package name, version, date, commit, and architecture
download_package() {
	package="$1"
	version_string="$2" # Now accepts full version string instead of separate components
	arch="$3"

	# If version_string contains '+', it's a full version identifier
	if [[ "$version_string" == *"+"* ]]; then
		file_name="${package}_${version_string}_${arch}.deb"
	else
		# Old format with separate components
		version="$version_string"
		date="$3"
		commit="$4"
		arch="$5"
		file_name="${package}_${version}-${date}-${commit}_${arch}.deb"
	fi

	# Try both repositories (with and without _dev suffix)
	for repo_suffix in "" "_dev"; do
		repo_url="https://repo.manticoresearch.com/repository/manticoresearch_jammy${repo_suffix}/dists/jammy/main/binary-amd64"
		file_url="${repo_url}/${file_name}"
		echo "Trying to download from $file_url"

		if wget -q --spider "$file_url" 2>/dev/null; then
			echo "Package found at $file_url"
			mkdir -p "../build"
			wget -q -O "../build/${file_name}" "$file_url"

			# For executor, we need to download the dev version and also extra package
			if [ "$package" = 'manticore-executor' ]; then
				if [[ "$version_string" == *"+"* ]]; then
					# Handle new format
					echo "Downloading executor dev package for new format version"
					# Extract version from version_string (before the +)
					version=$(echo "$version_string" | cut -d'+' -f1)
					# Extract date-commit from version_string (after the +)
					date_commit=$(echo "$version_string" | cut -d'+' -f2)

					echo "Wgetting from https://github.com/manticoresoftware/executor/releases/download/${version}/manticore-executor-${version}+${date_commit}-linux-amd64-dev.tar.gz"
					wget -q -O 'manticore-executor-dev.tar.gz' "https://github.com/manticoresoftware/executor/releases/download/${version}/manticore-executor-${version}+${date_commit}-linux-amd64-dev.tar.gz"
				else
					# Handle old format
					echo "Wgetting from https://github.com/manticoresoftware/executor/releases/download/v${version}/manticore-executor_${version}-${date}-${commit}_linux_amd64-dev.tar.gz"
					wget -q -O 'manticore-executor-dev.tar.gz' "https://github.com/manticoresoftware/executor/releases/download/v${version}/manticore-executor_${version}-${date}-${commit}_linux_amd64-dev.tar.gz"
				fi

				tar -xzf 'manticore-executor-dev.tar.gz'

				# Find the extracted directory
				executor_dev_dir=$(find . -type d -name "manticore-executor-*-linux-amd64-dev" | head -n 1)
				executor_dev_path=$(realpath "${executor_dev_dir}/manticore-executor")

				# Also add extra package
				if [[ "$version_string" == *"+"* ]]; then
					download_package "manticore-extra" "${version_string}" "all"
				else
					download_package "manticore-extra" "${version}" "${date}" "${commit}" "all"
				fi
			fi

			return 0
		fi
	done

	echo "ERROR: Package not found in any repository: ${file_name}" >&2
	exit 1
}

# Read deps.txt line by line
while read -r line
do
	echo "Processing dependency: $line"

	# Break if the line contains "---"
	if [[ $line == "---" ]]; then
		break
	fi

    if [[ "$line" =~ ^([^[:space:]]+)[[:space:]]+([^[:space:]]+)\+([0-9]+)-([a-f0-9]+)(-?[a-zA-Z0-9]+)?$ ]]; then
        # Format: <package> <version>+<date>-<commit>[-<optional_suffix>]
        package="${BASH_REMATCH[1]}"
        version="${BASH_REMATCH[2]}"
        date="${BASH_REMATCH[3]}"
        commit="${BASH_REMATCH[4]}"
        version_string="${version}+${date}-${commit}"
        if [ -n "${BASH_REMATCH[5]}" ]; then
            version_string="${version_string}${BASH_REMATCH[5]}"
        fi
        suffix="${BASH_REMATCH[5]}" # Optional suffix without leading "-"
        echo "Parsed new format:"
        echo "  Package: $package"
        echo "  Version: $version"
        echo "  Date: $date"
        echo "  Commit: $commit"
        echo "  Suffix: $suffix"
        echo "  Full version string: $version_string"
    elif [[ "$line" =~ ^([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]+([^[:space:]]+)$ ]]; then
        # Old format: <package> <version> <date> <commit>
        package="${BASH_REMATCH[1]}"
        version="${BASH_REMATCH[2]}"
        date="${BASH_REMATCH[3]}"
        commit="${BASH_REMATCH[4]}"
        suffix=""
        echo "Parsed old format:"
        echo "  Package: $package"
        echo "  Version: $version"
        echo "  Date: $date"
        echo "  Commit: $commit"
    else
        echo "ERROR: Unable to parse line: $line" >&2
        exit 1
    fi

	# Translate package names to the ones used in the repository
	case $package in
		buddy)
			# We use github clone instead of package
			# package="manticore-buddy"
			# arch="all"
			buddy_commit=$commit
			continue
			;;
		backup)
			package="manticore-backup"
			arch="all"
			;;
		load)
			package="manticore-load"
			arch="all"
			;;
		mcl)
			package="manticore-columnar-lib"
			arch="amd64"
			;;
		tzdata)
			package="manticore-tzdata"
			arch="all"
			;;
		executor)
			package="manticore-executor"
			arch="amd64"
			;;
		*)
			echo "Unknown package: $package"
			continue
			;;
	esac

	if [[ "$line" =~ ^([^[:space:]]+)[[:space:]]+([^[:space:]]+)\+([0-9]+)-([a-f0-9]+) ]]; then
		download_package "${package}" "${version_string}" "${arch}"
	else
		download_package "${package}" "${version}" "${date}" "${commit}" "${arch}"
	fi
done < ../deps.txt

# we want to build the image based on specific packages, copying them from a directory coming from an artifact of a previous job
deb_dir=$(realpath ../build/)

# Use official docker image to create and tune our test kit
docker pull --platform linux/amd64 manticoresearch/manticore:dev
docker rm -f manticore-test-kit >/dev/null 2>&1 || true
docker create \
	-v "$deb_dir:/build" --name manticore-test-kit \
	--entrypoint tail \
	--platform linux/amd64 \
	manticoresearch/manticore:dev \
	-f /dev/null
	docker start manticore-test-kit

	# Copy current repo into /manticore, excluding heavy/unused dirs to save space.
	docker exec manticore-test-kit mkdir -p /manticore
	tar -C .. --exclude=.git --exclude=manual --exclude=src --exclude=build* --exclude=artifact.tar --exclude=build --exclude=docker -cf - . | \
		docker exec -i manticore-test-kit tar -C /manticore -xf -

	docker cp "$executor_dev_path" manticore-test-kit:/usr/bin/manticore-executor-dev
	docker exec manticore-test-kit ln -sf /usr/bin/manticore-executor-dev /usr/bin/php

# Let's list what's in the /build/ inside the container for debug purposes
docker exec manticore-test-kit bash -c \
	'echo "Removing /build/manticore_*, because it may depend on manticore-buddy of a newer version while we'\''re installing Buddy via git clone"; rm /build/manticore_*.deb; ls -la /build/'

# Install deps and add manticore-executor-dev to the container
docker exec manticore-test-kit bash -c \
	'echo "apt list before update" && apt list --installed|grep manticore && apt-get -y update && echo "apt list after update" && apt list --installed|grep manticore && apt-get -y install manticore-galera && apt-get -y remove manticore-repo manticore && rm /etc/apt/sources.list.d/manticoresearch.list && apt-get update -y && dpkg -i --force-confnew /build/*.deb && \
	printf "#!/bin/sh\nexit 101\n" > /usr/sbin/policy-rc.d && chmod +x /usr/sbin/policy-rc.d && \
	DEBIAN_FRONTEND=noninteractive apt-get install -y libxml2 libcurl4 libonig5 libzip4 librdkafka1 curl neovim git apache2-utils iproute2 bash mariadb-server && \
	rm -f /usr/sbin/policy-rc.d && apt-get clean -y'

docker exec manticore-test-kit bash -c "cat /etc/manticoresearch/manticore.conf"

# Install composer cuz we need it for buddy from the git and also development
docker exec manticore-test-kit bash -c \
	"curl -sSL https://getcomposer.org/download/2.7.0/composer.phar > /usr/bin/composer; chmod +x /usr/bin/composer"

echo "Installing custom buddy from git repo with commit $buddy_commit"
# Install custom buddy from git repo
#
buddy_path=/usr/share/manticore/modules/manticore-buddy
docker exec manticore-test-kit bash -c \
	"rm -fr $buddy_path && \
	git clone https://github.com/manticoresoftware/manticoresearch-buddy.git $buddy_path && \
	git config --global --add safe.directory $buddy_path && \
	cd $buddy_path && \
	git checkout $buddy_commit && \
	composer install"

docker exec manticore-test-kit bash -c "grep -q 'query_log = /var/log/manticore/query.log' /etc/manticoresearch/manticore.conf || sed -i '/listen = 127.0.0.1:9308:http/a\    query_log = /var/log/manticore/query.log' /etc/manticoresearch/manticore.conf"
docker exec manticore-test-kit bash -c "grep -q 'log = /var/log/manticore/searchd.log' /etc/manticoresearch/manticore.conf || sed -i '/listen = 127.0.0.1:9308:http/a\    log = /var/log/manticore/searchd.log' /etc/manticoresearch/manticore.conf"

docker exec manticore-test-kit bash -c "cat /etc/manticoresearch/manticore.conf"

docker exec manticore-test-kit bash -c \
	'# Prepare MariaDB server for ubertests (db/user "test", empty password, federated enabled).
	set -e
	mkdir -p /var/run/mysqld
	chown -R mysql:mysql /var/run/mysqld
	if [ -f /etc/mysql/mariadb.conf.d/50-server.cnf ]; then
		if ! grep -q "^[[]mariadb[]]" /etc/mysql/mariadb.conf.d/50-server.cnf; then
			echo "[mariadb]" >> /etc/mysql/mariadb.conf.d/50-server.cnf
		fi
		if ! grep -q "^plugin_load_add[[:space:]]*=[[:space:]]*ha_federatedx" /etc/mysql/mariadb.conf.d/50-server.cnf; then
			echo "plugin_load_add = ha_federatedx" >> /etc/mysql/mariadb.conf.d/50-server.cnf
		fi
	fi
	service mariadb start >/dev/null 2>&1 || mysqld_safe --datadir=/var/lib/mysql &
	for i in $(seq 1 30); do
		if mysqladmin ping -h localhost --silent; then break; fi
		sleep 2
	done
	mysql -uroot -e "CREATE DATABASE IF NOT EXISTS test;"
	mysql -uroot test -e "CREATE USER IF NOT EXISTS '\''test'\''@'\''%'\'' IDENTIFIED BY '\'''\''; GRANT ALL PRIVILEGES ON test.* TO '\''test'\''@'\''%'\''; GRANT ALL PRIVILEGES ON *.* TO '\''test'\''@'\''%'\'' WITH GRANT OPTION; FLUSH PRIVILEGES;"'

docker exec manticore-test-kit bash -c \
    "md5sum /etc/manticoresearch/manticore.conf | awk '{print \$1}' > /manticore.conf.md5"

docker exec manticore-test-kit bash -c "rm -rf /tmp/*"

echo "Exporting image to ../manticore_test_kit.img"

# exporting the image, it also squashes all the layers into one
docker export manticore-test-kit > ../manticore_test_kit.img
