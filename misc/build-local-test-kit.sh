# Handle --clean parameter
if [ "$1" = "--clean" ]; then
	rm -rf ../build-for-test-kit/
fi

mkdir -p ../build-for-test-kit
docker run -it --rm \
-v $(pwd)/../cache:/cache \
-e CACHEB="/cache" \
-e DIAGNOSTIC=1 \
-e PACK_ICUDATA=0 \
-e NO_TESTS=1 \
-e DISTR=jammy \
-e boost=boost_nov22 \
-e sysroot=roots_nov22 \
-e arch=x86_64 \
-e CTEST_CMAKE_GENERATOR=Ninja \
-e CTEST_CONFIGURATION_TYPE=RelWithDebInfo \
-e WITH_COVERAGE=0 \
-e SYSROOT_URL="https://repo.manticoresearch.com/repository/sysroots" \
-e HOMEBREW_PREFIX="" \
-e PACK_GALERA=0 \
-e UNITY_BUILD=1 \
-v $(pwd)/../:/manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa \
manticoresearch/external_toolchain:vcpkg331_20250114 bash -c 'cd /manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/build-for-test-kit && cmake -DDISTR=jammy .. -DCMAKE_BUILD_TYPE=RelWithDebInfo && export CMAKE_TOOLCHAIN_FILE=$(pwd)/dist/build_dockers/cross/linux.cmake && cmake --build .'

# Build Docker image with compiled binaries
if [ -f ../build-for-test-kit/src/searchd ] && [ -f ../build-for-test-kit/src/indexer ] && [ -f ../build-for-test-kit/src/indextool ]; then
	cat > ../build-for-test-kit/Dockerfile <<EOF
FROM ghcr.io/manticoresoftware/manticoresearch:test-kit-latest
COPY src/searchd /usr/bin/searchd
COPY src/indexer /usr/bin/indexer
COPY src/indextool /usr/bin/indextool
EOF
	docker build --load -t test-kit ../build-for-test-kit
	rm -f ../build-for-test-kit/Dockerfile
	echo "Docker image 'test-kit' created successfully"
else
	echo "Error: Compiled binaries not found. Cannot create Docker image."
	exit 1
fi
