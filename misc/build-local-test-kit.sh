#!/bin/bash
# Build a local test-kit Docker image with your code changes
#
# This script compiles Manticore Search and creates a Docker image for CLT testing.
# The image is based on test-kit-latest (contains latest dev from master) with your
# compiled binaries replacing the originals.
#
# Usage:
#   ./build-local-test-kit.sh [--clean] [--help]
#
# Options:
#   --clean    Remove build directory before building
#   --help     Show this help message
#
# Requirements:
#   - Docker installed and running
#   - At least 10GB free disk space
#   - Internet connection (to pull images)
#
# Example:
#   cd misc
#   ./build-local-test-kit.sh --clean

set -e  # Exit on error

# Show help
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    head -n 23 "$0" | tail -n 19
    exit 0
fi

# Configuration
TOOLCHAIN_IMAGE="manticoresearch/external_toolchain:vcpkg331_20250114"
BASE_IMAGE="ghcr.io/manticoresoftware/manticoresearch:test-kit-latest"
OUTPUT_IMAGE="test-kit:local"
BUILD_TYPE="${BUILD_TYPE:-RelWithDebInfo}"

# Determine paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"

# Cleanup on exit
cleanup() {
    if [ -f "$BUILD_DIR/Dockerfile" ]; then
        rm -f "$BUILD_DIR/Dockerfile"
    fi
}
trap cleanup EXIT INT TERM

echo "==> Building local test-kit image"
echo "    Repository: $REPO_ROOT"
echo "    Build type: $BUILD_TYPE"
echo ""

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
    echo "==> Removing build directory for clean build..."
    rm -rf "$BUILD_DIR"
fi

# Create directories
mkdir -p "$BUILD_DIR"
mkdir -p "$REPO_ROOT/cache"

# Pull base image (contains latest dev from master)
echo "==> Pulling base image: $BASE_IMAGE"
if ! docker pull "$BASE_IMAGE"; then
    echo "ERROR: Failed to pull base image. Check your internet connection."
    exit 1
fi

# Step 1: Compile Manticore Search
echo ""
echo "==> Step 1/2: Compiling Manticore Search"
echo "    This may take 10-30 minutes..."
echo "    Press Ctrl+C to cancel"
echo ""

# Note: The long path (64 'a' chars) is required for CMake compatibility
# with path length limitations on some platforms. Do not change it.
MOUNT_PATH="/manticore_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

docker run --rm \
    --platform linux/amd64 \
    -v "$REPO_ROOT/cache:/cache" \
    -e CACHEB="/cache" \
    -e DIAGNOSTIC=1 \
    -e PACK_ICUDATA=0 \
    -e NO_TESTS=1 \
    -e DISTR=jammy \
    -e boost=boost_nov22 \
    -e sysroot=roots_nov22 \
    -e arch=x86_64 \
    -e CTEST_CMAKE_GENERATOR=Ninja \
    -e CTEST_CONFIGURATION_TYPE="$BUILD_TYPE" \
    -e WITH_COVERAGE=0 \
    -e SYSROOT_URL="https://repo.manticoresearch.com/repository/sysroots" \
    -e HOMEBREW_PREFIX="" \
    -e PACK_GALERA=0 \
    -e UNITY_BUILD=1 \
    -v "$REPO_ROOT:$MOUNT_PATH" \
    "$TOOLCHAIN_IMAGE" \
    bash -c "cd $MOUNT_PATH/build && \
             cmake -DDISTR=jammy .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE && \
             export CMAKE_TOOLCHAIN_FILE=\$(pwd)/dist/build_dockers/cross/linux.cmake && \
             cmake --build ."

# Step 2: Create Docker image with compiled binaries
echo ""
echo "==> Step 2/2: Creating Docker image"

# Verify binaries exist
if [ ! -f "$BUILD_DIR/src/searchd" ] || [ ! -f "$BUILD_DIR/src/indexer" ] || [ ! -f "$BUILD_DIR/src/indextool" ]; then
    echo "ERROR: Compiled binaries not found. Build may have failed."
    echo "Try running with --clean for a fresh build."
    exit 1
fi

# Create Dockerfile
cat > "$BUILD_DIR/Dockerfile" <<EOF
FROM $BASE_IMAGE
COPY src/searchd /usr/bin/searchd
COPY src/indexer /usr/bin/indexer
COPY src/indextool /usr/bin/indextool
RUN chmod +x /usr/bin/searchd /usr/bin/indexer /usr/bin/indextool
EOF
	docker build --load -t test-kit ../build-for-test-kit
	rm -f ../build-for-test-kit/Dockerfile
	echo "Docker image 'test-kit' created successfully"
else
	echo "Error: Compiled binaries not found. Cannot create Docker image."
	exit 1
fi

# Build Docker image
docker build --load -t "$OUTPUT_IMAGE" "$BUILD_DIR"

echo ""
echo "==> SUCCESS! Docker image '$OUTPUT_IMAGE' created"
echo ""
echo "Next steps:"
echo "  1. Test your image:"
echo "     docker run --rm $OUTPUT_IMAGE searchd --version"
echo ""
echo "  2. Run CLT tests:"
echo "     cd /path/to/clt"
echo "     ./clt test -t /path/to/test.rec -d $OUTPUT_IMAGE"
echo ""
echo "For more details, see TESTING.md"
