#!/bin/bash
set -e


git clone https://github.com/manticoresoftware/docker.git docker
cd docker

repo_urls=("https://repo.manticoresearch.com/repository/manticoresearch_jammy_dev/dists/jammy/main/binary-amd64/"
		   "https://repo.manticoresearch.com/repository/manticoresearch_jammy/dists/jammy/main/binary-amd64/")

# Fetch the pages from each repository and store them in temporary files
for index in "${!repo_urls[@]}"; do
	curl -s "${repo_urls[$index]}" > "/tmp/packages_${index}"
done

# We set it later when parse deps.txt
executor_dev_path=

# Read deps.txt line by line
while read -r line
do
  echo "Processing dependency: $line"

	# Break if the line contains "---"
	if [[ $line == "---" ]]; then
		break
	fi

	package=$(echo $line | awk '{print $1}') # First column is package
	version=$(echo $line | awk '{print $2}') # Second column is version
	date=$(echo $line | awk '{print $3}')    # Third column is date
	commit=$(echo $line | awk '{print $4}')  # Fourth column is commit

	last_digit=$(echo $version | awk -F. '{print $NF}')

	if [ $((last_digit%2)) -eq 0 ]; then
	   # is_release_version=1
	   repo_suffix=
	else
	   # is_release_version=0
	   repo_suffix="_dev"
	fi


	# Translate package names to the ones used in the repository
	case $package in
		buddy)
			package="manticore-buddy"
			;;
		backup)
			package="manticore-backup"
			;;
		mcl)
			package="manticore-columnar-lib"
			;;
    tzdata)
			package="manticore-tzdata"
			;;
		executor)
			package="manticore-executor|manticore-extra"
			;;
		*)
			echo "Unknown package: $package"
			continue
			;;
	esac

	# Search each downloaded file and find the line with the specific package, version, date, and commit
	for index in "${!repo_urls[@]}"; do
		if [ "$package" = 'manticore-executor|manticore-extra' ]; then
      		echo "Wgetting from https://github.com/manticoresoftware/executor/releases/download/v${version}/manticore-executor_${version}-${date}-${commit}_linux_amd64-dev.tar.gz"
			wget -q -O 'manticore-executor-dev.tar.gz' "https://github.com/manticoresoftware/executor/releases/download/v${version}/manticore-executor_${version}-${date}-${commit}_linux_amd64-dev.tar.gz"
			tar -xzf 'manticore-executor-dev.tar.gz'
			executor_dev_path=$(realpath "manticore-executor_${version}-${date}-${commit}_linux_amd64-dev/manticore-executor")
		fi

		cat "/tmp/packages_${index}" | egrep "$package" | grep $commit | grep ".deb" | awk -v repo_url="${repo_urls[$index]}" -F\" '{print repo_url""$2"?ci=1"}' |
		while read file_url
		do
			file_name=$(basename "$file_url" "?ci=1")
      echo "Wgetting from $file_url"
			wget -q -O "../build/$file_name" "$file_url"
		done
	done

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
  'ls -la /build/'
# Install deps and add manticore-executor-dev to the container
docker exec manticore-test-kit bash -c \
	"apt-get -y update && apt-get -y install manticore-galera && apt-get -y remove 'manticore-repo' && rm /etc/apt/sources.list.d/manticoresearch.list && apt-get update -y && apt-get install -y --allow-downgrades /build/*.deb libxml2 libcurl4 libonig5 libzip4 librdkafka1 curl neovim git apache2-utils iproute2 bash && apt-get clean -y"
docker exec manticore-test-kit bash -c \
	"curl -sSL https://getcomposer.org/download/2.7.0/composer.phar > /usr/bin/composer; chmod +x /usr/bin/composer"

echo "Exporting image to ../manticore_test_kit.img"

# exporting the image, it also squashes all the layers into one
docker export manticore-test-kit > ../manticore_test_kit.img
