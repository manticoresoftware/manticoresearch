#!/bin/bash
#Run shell in valgrind docker

BUILD_DOCKER=manticore_valgrind:jammy
CONFIG=RelWithDebInfo
CACHEB=../cache
WORKDIR=/work
uid=`id -u`

docker run --rm -it \
-v $(pwd):$WORKDIR \
-e uid=$uid \
-e DIAGNOSTIC=1 \
-e VERBOSE=1 \
-e CACHEB=$CACHEB \
-e CTEST_CONFIGURATION_TYPE=$CONFIG \
-e UNITY_BUILD=1 \
$BUILD_DOCKER /bin/bash