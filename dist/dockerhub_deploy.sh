#!/bin/bash

for ((i = 0; i < 3; i++)); do
     echo "Trying to authorise into dockerhub"
      echo $DOCKER_PASSWORD|docker login -u$DOCKER_USER --password-stdin docker.io
      docker info | grep Username|grep $DOCKER_USER && break
done

# Login to ghcr.io with provided token and username

echo $GHCR_PASSWORD | docker login -u$GHCR_USER --password-stdin ghcr.io

if ! (docker info | grep Username) > /dev/null 2>&1; then
      echo "Can't authorise to dockerhub"
      exit 1
fi

if [ ! -n "$BUILD_TAGS" ]; then
  BUILD_TAGS="dev dev-$( cat src/sphinxversion.h.in | grep VERNUMBERS | cut -d" " -f3 | cut -d'"' -f2 )-$CI_COMMIT_SHORT_SHA"
fi

IFS=' ' read -r -a SPLITTED_BUILD_TAGS <<<"$BUILD_TAGS"

git clone https://github.com/manticoresoftware/docker.git docker
cd docker

BUILD_FAILED=false

if [[ ! $(docker ps | grep manticore_build) ]]; then
    echo "Buildx builder not found. Try to run it manually"
    docker buildx create  --name manticore_build --platform linux/amd64,linux/arm64
    docker buildx use manticore_build
    docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
fi

for BUILD_TAG in "${SPLITTED_BUILD_TAGS[@]}"; do
    for ((i = 0; i < 3; i++)); do
      echo "Started to buid manticoresearch/manticore:$BUILD_TAG"

      docker buildx build --build-arg DEV=1 --push --platform linux/arm64,linux/amd64 --tag  manticoresearch/manticore:$BUILD_TAG . && break

      if [ $i==2 ]; then
        echo "Docker build are failed"
        BUILD_FAILED=true
      fi
    done

    # CLT kit image to run tests based on official image
    echo -e "common { \n\
        plugin_dir = /usr/local/lib/manticore\n\
        lemmatizer_base = /usr/share/manticore/morph/\n\
    }\n\
    searchd {\n\
        listen = 9306:mysql41\n\
        listen = /var/run/mysqld/mysqld.sock:mysql41\n\
        listen = 9312\n\
        listen = 9308:http\n\
        log = /var/log/manticore/searchd.log\n\
        query_log = /var/log/manticore/query.log\n\
        pid_file = /var/run/manticore/searchd.pid\n\
        data_dir = /var/lib/manticore\n\
        query_log_format = sphinxql\n\
        # buddy_path = manticore-executor-dev /workdir/src/main.php\n\
    }\n" > manticore.conf
    docker rm -f manticore-with-extra || true
    docker create --name manticore-with-extra manticoresearch/manticore:$BUILD_TAG
    docker start manticore-with-extra
    docker exec -e EXTRA=1 manticore-with-extra /entrypoint.sh
    docker cp manticore.conf manticore-with-extra:/etc/manticoresearch/manticore.conf
    docker commit manticore-with-extra ghcr.io/manticoresoftware/manticoresearch:clt-$BUILD_TAG
    docker push ghcr.io/manticoresoftware/manticoresearch:clt-$BUILD_TAG
done

echo "Done"
cd ..
rm -rf docker

if [ "$BUILD_FAILED" = true ] ; then
    exit 1
fi
