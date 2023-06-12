#!/bin/bash

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

repo_urls=("https://repo.manticoresearch.com/repository/manticoresearch_focal_dev/dists/focal/main/binary-amd64/"
           "https://repo.manticoresearch.com/repository/manticoresearch_focal/dists/focal/main/binary-amd64/")

# Fetch the pages from each repository and store them in temporary files
for index in "${!repo_urls[@]}"; do
    curl -s "${repo_urls[$index]}" > "/tmp/packages_${index}"
done

# Read deps.txt line by line
while read -r line
do
    # Break if the line contains "---"
    if [[ $line == "---" ]]; then
        break
    fi

    package=$(echo $line | awk '{print $1}') # First column is package
    version=$(echo $line | awk '{print $2}') # Second column is version
    date=$(echo $line | awk '{print $3}')    # Third column is date
    commit=$(echo $line | awk '{print $4}')  # Fourth column is commit

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
        cat "/tmp/packages_${index}" | grep $package | grep $commit | grep ".deb" | awk -v repo_url="${repo_urls[$index]}" -F\" '{print repo_url""$2"?ci=1"}' |
        while read file_url
        do
            file_name=$(basename "$file_url" "?ci=1")
            wget -q -O "$file_name" "$file_url"
        done
    done

done < ../deps.txt

# we want to build the image based on specific packages, copying them from a directory coming from an artifact of a previous job
cp ../build/*.deb .

# adding manticore.conf to the image
docker buildx create --use
docker buildx build --progress=plain --build-arg DEV=0 --build-arg DAEMON_URL="" --build-arg  MCL_URL="" --load --platform linux/amd64 --tag manticore:clt .
docker create --name manticore-clt manticore:clt
docker start manticore-clt
docker cp manticore.conf manticore-clt:/etc/manticoresearch/manticore.conf

img_url="ghcr.io/manticoresoftware/manticoresearch:$BUILD_TAG"
# exporting the image, it also squashes all the layers into one
docker export manticore-clt > manticore_clt.img
docker import manticore_clt.img $img_url

# pusing to ghcr.io
docker push $img_url \
  && echo "❗ Pushed the image to $img_url" \
  || echo "❗ Couldn't push the image to $img_url"
