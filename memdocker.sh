#!/bin/bash
#Run shell in valgrind docker

# Use environment variable BUILD_DOCKER if set, otherwise use default value
BUILD_DOCKER=${BUILD_DOCKER:-manticoresearch/manticore_valgrind:jammy}
CONFIG=RelWithDebInfo
WORKDIR=/work

docker run --rm -it \
-v $(pwd):$WORKDIR \
-e uid=`id -u` \
-e gid=`id -g` \
-e DIAGNOSTIC=1 \
-e VERBOSE=1 \
-e CACHEB=$CACHEB \
-e CTEST_CONFIGURATION_TYPE=$CONFIG \
-e UNITY_BUILD=1 \
$BUILD_DOCKER /bin/bash