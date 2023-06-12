#!/bin/bash

for ((i = 0; i < 3; i++)); do
    echo "Trying to log in to Docker Hub"
    echo $DOCKER_PASSWORD|docker login -u$DOCKER_USER --password-stdin docker.io && break
    FAILED_TO_LOGIN=true
done

if [[ "$FAILED_TO_LOGIN" == 'true' ]]; then
      echo "Can't log in to Docker Hub"
      exit 1
fi

if [ ! -n "$BUILD_TAGS" ]; then
  BUILD_TAGS="dev-tmp dev-tmp-$( cat src/sphinxversion.h.in | grep VERNUMBERS | cut -d" " -f3 | cut -d'"' -f2 )-$CI_COMMIT_SHORT_SHA"
fi

IFS=' ' read -r -a SPLITTED_BUILD_TAGS <<<"$BUILD_TAGS"

git clone https://github.com/manticoresoftware/docker.git docker
cd docker

BUILD_FAILED=false

if [[ ! $(docker ps | grep manticore_build) ]]; then
    echo "Buildx builder not found. Registering."
    docker buildx create --name manticore_build --platform linux/amd64,linux/arm64
    docker buildx use manticore_build
#    docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
fi

for BUILD_TAG in "${SPLITTED_BUILD_TAGS[@]}"; do
    for ((i = 0; i < 3; i++)); do
      echo "Started building manticoresearch/manticore:$BUILD_TAG"

      docker buildx build --progress=plain --build-arg DEV=1 --push --platform linux/arm64,linux/amd64 --tag manticoresearch/manticore:$BUILD_TAG . && break

      if [ $i==2 ]; then
        echo "Docker build failed"
        BUILD_FAILED=true
      fi
    done
done

echo "Done"
cd ..
rm -rf docker

if [ "$BUILD_FAILED" = true ] ; then
    exit 1
fi
