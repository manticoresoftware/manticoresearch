#!/bin/bash

#Run the very same test suite as run on gitlab CI, but locally

BUILD_DOCKER=manticoresearch/external_toolchain:vcpkg331_20250114
TEST_DOCKER=manticoresearch/ubertests_public:331
boost=boost_nov22
sysroot=roots_nov22
CONFIG=Debug
CACHEB=../cache
SYSROOT="-e SYSROOT_URL=https://repo.manticoresearch.com/repository/sysroots"
if test -d /work/cross/sysroots/checked; then
  SYSROOT="-e SYSROOT_URL=file:///sysroots/ -v /work/cross/sysroots/checked:/sysroots"
elif test -d /mnt/repo_storage/sysroots; then
  SYSROOT="-e SYSROOT_URL=file:///sysroots/ -v /mnt/repo_storage/sysroots:/sysroots"
fi

build()
{
CONFIG=$1
BUILD_DIR=build_$CONFIG
docker run --rm \
-v $(pwd):/build/dev \
-e uid=`id -u` \
-e gid=`id -g` \
$SYSROOT \
-e DISTR=jammy \
-e arch=x86_64 \
-e boost=$boost \
-e sysroot=$sysroot \
-e DIAGNOSTIC=1 \
-e VERBOSE=1 \
-e CACHEB=$CACHEB \
-e BUILD_DIR=$BUILD_DIR \
-e CTEST_CMAKE_GENERATOR=Ninja \
-e CTEST_CONFIGURATION_TYPE=$CONFIG \
-e UNITY_BUILD=1 \
-e NO_TESTS=1 \
$BUILD_DOCKER /bin/bash -c '
trap "chown -R $uid:$gid /build/dev/$BUILD_DIR" EXIT
cd /build/dev
rm -rf $BUILD_DIR
ctest -VV -S misc/ctest/gltest.cmake
cd $BUILD_DIR
chmod -R a+rw $CACHEB
tar -cf build.tar api/libsphinxclient/testcli src/indexer src/indextool src/searchd src/gtests/gmanticoretest
find . -name *.h -o -name *.c -o -name *.cxx -o -name *.gcno | tar -rf build.tar -T-
mv build.tar /tmp
rm -rf *
rm .ninja_deps
rm .ninja_log
tar -xf /tmp/build.tar
rm /tmp/build.tar
'
}

test ()
{
CONFIG=$1
BUILD_DIR=build_$CONFIG
docker run --rm \
-v $(pwd):/build/dev \
-e uid=`id -u` \
-e gid=`id -g` \
$SYSROOT \
-e DISTR=jammy \
-e arch=x86_64 \
-e boost=$boost \
-e sysroot=$sysroot \
-e DIAGNOSTIC=1 \
-e VERBOSE=1 \
-e CACHEB=$CACHEB \
-e BUILD_DIR=$BUILD_DIR \
-e CTEST_CMAKE_GENERATOR=Ninja \
-e CTEST_CONFIGURATION_TYPE=$CONFIG \
-e UNITY_BUILD=1 \
-e NO_BUILD=1 \
$TEST_DOCKER /bin/bash -c '
cd /build/dev
trap "chown -R $uid:$gid /build/dev/$BUILD_DIR" EXIT
ctest -V -S misc/ctest/gltest.cmake --no-compress-output
'
}

shell ()
{
CONFIG=$1
BUILD_DIR=build_$CONFIG
docker run -it --rm \
-v $(pwd):/build/dev \
$SYSROOT \
-e DISTR=jammy \
-e arch=x86_64 \
-e boost=$boost \
-e sysroot=$sysroot \
-e DIAGNOSTIC=1 \
-e VERBOSE=1 \
-e CACHEB=$CACHEB \
-e BUILD_DIR=$BUILD_DIR \
-e CTEST_CMAKE_GENERATOR=Ninja \
-e CTEST_CONFIGURATION_TYPE=$CONFIG \
-e UNITY_BUILD=1 \
-e NO_BUILD=1 \
$TEST_DOCKER /bin/bash
}

build $CONFIG
time test $CONFIG
#shell $CONFIG