# Handle --clean parameter
if [ "$1" = "--clean" ]; then
	rm -rf ../build
        rm -fr ../cache/linux-x86_64/mcl/*
fi

mkdir -p ../build
if ! docker run --platform linux/amd64 -it --rm \
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
-v $(pwd)/../../columnar/:/columnar \
-v $(pwd)/../:/manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa \
manticoresearch/external_toolchain:vcpkg331_20250114 bash -c 'echo "SOURCE_DIR /columnar" > /manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/local_columnar_src.txt && cd /manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/build && cmake -DDISTR=jammy .. -DCMAKE_BUILD_TYPE=Debug && export CMAKE_TOOLCHAIN_FILE=$(pwd)/dist/build_dockers/cross/linux.cmake && cmake --build .'; then
	echo "Error: Docker build failed. Aborting."
	exit 1
fi

# Build Docker image with compiled binaries
cp ../../columnar/build/knn/lib_manticore_knn.so ../build/lib_manticore_knn.so
cp ../../columnar/build/knn/embeddings/release/lib_manticore_knn_embeddings.so ../build/lib_manticore_knn_embeddings.so

if [ -f ../build/src/searchd ] && [ -f ../build/src/indexer ] && [ -f ../build/src/indextool ]; then
	cat > ../build/Dockerfile <<EOF
FROM ghcr.io/manticoresoftware/manticoresearch:test-kit-latest
COPY . /manticore/
COPY build/src/searchd /usr/bin/searchd
COPY build/src/indexer /usr/bin/indexer
COPY build/src/indextool /usr/bin/indextool
COPY build/lib_manticore_knn.so /usr/share/manticore/modules/lib_manticore_knn.so
COPY build/lib_manticore_knn_embeddings.so /usr/share/manticore/modules/lib_manticore_knn_embeddings.so
EOF
	docker build --load -t test-kit -f ../build/Dockerfile ..
	rm -f ../build/Dockerfile
	echo "Docker image 'test-kit' created successfully"
else
	echo "Error: Compiled binaries not found. Cannot create Docker image."
	exit 1
fi
