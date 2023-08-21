#!/bin/bash
set -e

echo $GHCR_PASSWORD | docker login -u$GHCR_USER --password-stdin ghcr.io

if ! (docker info | grep Username) > /dev/null 2>&1; then
  echo "Can't authorise to GHCR docker registry"
  exit 1
fi

git clone https://github.com/manticoresoftware/docker.git docker
cd docker

cat > manticore.conf << EOF
common {
	plugin_dir = /usr/local/lib/manticore
	lemmatizer_base = /usr/share/manticore/morph/
}
searchd {
	listen = 9306:mysql41
	listen = /var/run/mysqld/mysqld.sock:mysql41
	listen = 9312
	listen = 9308:http
	log = /var/log/manticore/searchd.log
	query_log = /var/log/manticore/query.log
	pid_file = /var/run/manticore/searchd.pid
	data_dir = /var/lib/manticore
	query_log_format = sphinxql
	# buddy_path = manticore-executor-dev /workdir/src/main.php
}
EOF

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
		executor)
			package="manticore-executor"
			;;
		*)
			echo "Unknown package: $package"
			continue
			;;
	esac

	# Search each downloaded file and find the line with the specific package, version, date, and commit
	for index in "${!repo_urls[@]}"; do
		if [ "$package" = 'manticore-executor' ]; then
      echo "Wgetting from https://github.com/manticoresoftware/executor/releases/download/v${version}/manticore-executor_${version}-${date:0:6}-${commit}_linux_amd64-dev.tar.gz"
			wget -q -O 'manticore-executor-dev.tar.gz' "https://github.com/manticoresoftware/executor/releases/download/v${version}/manticore-executor_${version}-${date:0:6}-${commit}_linux_amd64-dev.tar.gz"
			tar -xzf 'manticore-executor-dev.tar.gz'
			executor_dev_path=$(realpath "manticore-executor_${version}-${date:0:6}-${commit}_linux_amd64-dev/manticore-executor")

			pwd
      echo "Wgetting from https://repo.manticoresearch.com/repository/manticoresearch_jammy${repo_suffix}/dists/jammy/main/binary-amd64/manticore-extra_${version}-${date}-${commit}_all.deb"
			wget -q -O '../build/manticore-extra.deb' \
				"https://repo.manticoresearch.com/repository/manticoresearch_jammy${repo_suffix}/dists/jammy/main/binary-amd64/manticore-extra_${version}-${date}-${commit}_all.deb"
			continue
		fi

		cat "/tmp/packages_${index}" | grep $package | grep $commit | grep ".deb" | awk -v repo_url="${repo_urls[$index]}" -F\" '{print repo_url""$2"?ci=1"}' |
		while read file_url
		do
			file_name=$(basename "$file_url" "?ci=1")
      echo "Wgetting from $file_url"
			wget -q -O "$file_name" "$file_url"
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

# Add modified config to it
docker cp manticore.conf manticore-test-kit:/etc/manticoresearch/manticore.conf

docker cp "$executor_dev_path" manticore-test-kit:/usr/bin/manticore-executor-dev
docker exec manticore-test-kit ln -sf /usr/bin/manticore-executor-dev /usr/bin/php
# Install deps and add manticore-executor-dev to the container
docker exec manticore-test-kit bash -c \
	'apt-get update -y && apt-get install -y --allow-downgrades /build/*.deb libxml2 libcurl4 libonig5 libzip4 curl neovim git apache2-utils iproute2 bash && apt-get clean -y'
docker exec manticore-test-kit bash -c \
	"php -r \"copy('https://getcomposer.org/installer', 'composer-setup.php');\" && php -r \"if (hash_file('sha384', 'composer-setup.php') === 'e21205b207c3ff031906575712edab6f13eb0b361f2085f1f1237b7126d785e826a450292b6cfd1d64d92e6563bbde02') { echo 'Installer verified'; } else { echo 'Installer corrupt'; unlink('composer-setup.php'); } echo PHP_EOL;\" && php composer-setup.php && php -r \"unlink('composer-setup.php');\" && mv composer.phar /usr/bin/composer || true"

img_url="ghcr.io/manticoresoftware/manticoresearch:test-kit-$BUILD_COMMIT"
img_url_latest="ghcr.io/manticoresoftware/manticoresearch:test-kit-latest"

# exporting the image, it also squashes all the layers into one
docker export manticore-test-kit > manticore_test_kit.img
docker import manticore_test_kit.img $img_url
docker tag $img_url $img_url_latest
# pusing to ghcr.io

images=("$img_url" "$img_url_latest")
for img in "${images[@]}"; do
	docker push $img \
	  && echo "❗ Pushed the image to $img" \
	  || echo "❗ Couldn't push the image to $img"
done
