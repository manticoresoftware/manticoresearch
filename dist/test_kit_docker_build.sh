#!/bin/bash
set -e

git clone https://github.com/manticoresoftware/docker.git docker
cd docker

# We set it later when parse deps.txt
executor_dev_path=

# Extra function to use
# Get semver suffix in format -dev or empty string depending on version provided
get_semver_suffix() {
	local version="$1"

	# Check if last digit is odd
	last_digit=$(echo "$version" | awk -F. '{print $NF}')
	if [ $(( last_digit % 2 )) -eq 0 ]; then
		echo ""
	else
		echo "_dev"
	fi
}

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

	file_url="${repo_url}/${file_name}"
	echo "Wgetting from $file_url"
	wget -q -O "../build/${file_name}" "$file_url"

	# For executor, we need to download the dev version and also extra package
	if [ "$package" = 'manticore-executor' ]; then
		echo "Wgetting from https://github.com/manticoresoftware/executor/releases/download/v${version}/manticore-executor_${version}-${date}-${commit}_linux_amd64-dev.tar.gz"
		wget -q -O 'manticore-executor-dev.tar.gz' "https://github.com/manticoresoftware/executor/releases/download/v${version}/manticore-executor_${version}-${date}-${commit}_linux_amd64-dev.tar.gz"
		tar -xzf 'manticore-executor-dev.tar.gz'
		executor_dev_path=$(realpath "manticore-executor_${version}-${date}-${commit}_linux_amd64-dev/manticore-executor")

		# Also add extra package
		download_package "manticore-extra" "${version}" "${date}" "${commit}" "all"
	fi
}

# Read deps.txt line by line
while read -r line
do
	echo "Processing dependency: $line"

	# Break if the line contains "---"
	if [[ $line == "---" ]]; then
		break
	fi

    if [[ "$line" =~ ^([^[:space:]]+)[[:space:]]+([^[:space:]]+)\+([0-9]+)-([a-f0-9]+)-?([a-zA-Z0-9]+)?$ ]]; then
        # Format: <package> <version>+<date>-<commit>-<optional_suffix>
        package="${BASH_REMATCH[1]}"
        version_string="${BASH_REMATCH[2]}+${BASH_REMATCH[3]}-${BASH_REMATCH[4]}"
        if [ -n "${BASH_REMATCH[5]}" ]; then
            version_string="${version_string}-${BASH_REMATCH[5]}"
        fi
        suffix="${BASH_REMATCH[5]}" # Optional suffix without leading "-"
    elif [[ "$line" =~ ^([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]+([^[:space:]]+)$ ]]; then
        # Old format: <package> <version> <date> <commit>
        package="${BASH_REMATCH[1]}"
        version="${BASH_REMATCH[2]}"
        date="${BASH_REMATCH[3]}"
        commit="${BASH_REMATCH[4]}"
        suffix=""
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

	# Update suffix logic
	if [ "$suffix" = "dev" ]; then
		repo_suffix="_dev"
	else
		repo_suffix=$(get_semver_suffix "$version")
	fi
	repo_url="https://repo.manticoresearch.com/repository/manticoresearch_jammy${repo_suffix}/dists/jammy/main/binary-amd64"

	if [[ "$line" =~ ^([^[:space:]]+)[[:space:]]+([^[:space:]]+)\+([0-9]+)-([a-f0-9]+) ]]; then
		download_package "${package}" "${version_string}" "${arch}"
	else
		download_package "${package}" "${version}" "${date}" "${commit}" "${arch}"
	fi
done < ../deps.txt

# we want to build the image based on specific packages, copying them from a directory coming from an artifact of a previous job
deb_dir=$(realpath ../build/)

# Use official docker image to create and tune our test kit
docker pull manticoresearch/manticore:dev
docker create \
	-v "$deb_dir:/build" --name manticore-test-kit \
	--entrypoint tail \
	manticoresearch/manticore:dev \
	-f /dev/null
	docker start manticore-test-kit

	docker cp "$executor_dev_path" manticore-test-kit:/usr/bin/manticore-executor-dev
	docker exec manticore-test-kit ln -sf /usr/bin/manticore-executor-dev /usr/bin/php

# Let's list what's in the /build/ inside the container for debug purposes
docker exec manticore-test-kit bash -c \
	'echo "Removing /build/manticore_*, because it may depend on manticore-buddy of a newer version while we'\''re installing Buddy via git clone"; rm /build/manticore_*.deb; ls -la /build/'
# Install deps and add manticore-executor-dev to the container
docker exec manticore-test-kit bash -c \
	"apt-get -y update && apt-get -y install manticore-galera && apt-get -y remove 'manticore-repo' && rm /etc/apt/sources.list.d/manticoresearch.list && apt-get update -y && apt-get install -y --allow-downgrades /build/*.deb libxml2 libcurl4 libonig5 libzip4 librdkafka1 curl neovim git apache2-utils iproute2 bash && apt-get clean -y"

# Install composer cuz we need it for buddy from the git and also development
docker exec manticore-test-kit bash -c \
	"curl -sSL https://getcomposer.org/download/2.7.0/composer.phar > /usr/bin/composer; chmod +x /usr/bin/composer"

# Install custom buddy from git repo
#
buddy_path=/usr/share/manticore/modules/manticore-buddy
docker exec manticore-test-kit bash -c \
	"rm -fr $buddy_path && \
	git clone https://github.com/manticoresoftware/manticoresearch-buddy.git $buddy_path && \
	git config --global --add safe.directory $buddy_path && \
	cd $buddy_path && \
	git checkout $buddy_commit && \
	composer install && \
	curl -sSL https://raw.githubusercontent.com/manticoresoftware/phar_builder/refs/heads/main/templates/sh | \
	sed 's/__NAME__/manticore-buddy/g; s/__PACKAGE__/manticore-buddy/g' >$buddy_path/bin/manticore-buddy && \
	chmod +x $buddy_path/bin/manticore-buddy"

echo "Exporting image to ../manticore_test_kit.img"

# exporting the image, it also squashes all the layers into one
docker export manticore-test-kit > ../manticore_test_kit.img
