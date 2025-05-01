#!/bin/bash
set -e

SCRIPT_DIR="$(dirname "$0")"
REPO_ROOT="$(realpath "$SCRIPT_DIR/..")"
TEMP_DIR=$(mktemp -d)

echo "Current directory: $(pwd)"
echo "Script location: $SCRIPT_DIR"
echo "Expected repository root: $REPO_ROOT"

map_package_name() {
  case "$1" in
    backup) echo "manticore-backup" ;;
    buddy) echo "manticore-buddy" ;;
    load) echo "manticore-load" ;;
    mcl) echo "manticore-columnar-lib" ;;
    tzdata) echo "manticore-tzdata" ;;
    executor) echo "manticore-executor" ;;
    read) echo "manticore-read" ;;
    grok) echo "manticore-grok" ;;
    *) echo "$1" ;;
  esac
}

urls=(
  "bionic-amd.html https://repo.manticoresearch.com/repository/manticoresearch_bionic_dev/dists/bionic/main/binary-amd64/"
  "bionic-arm.html https://repo.manticoresearch.com/repository/manticoresearch_bionic_dev/dists/bionic/main/binary-arm64/"
  "bookworm-amd.html https://repo.manticoresearch.com/repository/manticoresearch_bookworm_dev/dists/bookworm/main/binary-amd64/"
  "bookworm-arm.html https://repo.manticoresearch.com/repository/manticoresearch_bookworm_dev/dists/bookworm/main/binary-arm64/"
  "bullseye-amd.html https://repo.manticoresearch.com/repository/manticoresearch_bullseye_dev/dists/bullseye/main/binary-amd64/"
  "bullseye-arm.html https://repo.manticoresearch.com/repository/manticoresearch_bullseye_dev/dists/bullseye/main/binary-arm64/"
  "buster-amd.html https://repo.manticoresearch.com/repository/manticoresearch_buster_dev/dists/buster/main/binary-amd64/"
  "buster-arm.html https://repo.manticoresearch.com/repository/manticoresearch_buster_dev/dists/buster/main/binary-arm64/"
  "focal-amd.html https://repo.manticoresearch.com/repository/manticoresearch_focal_dev/dists/focal/main/binary-amd64/"
  "focal-arm.html https://repo.manticoresearch.com/repository/manticoresearch_focal_dev/dists/focal/main/binary-arm64/"
  "jammy-amd.html https://repo.manticoresearch.com/repository/manticoresearch_jammy_dev/dists/jammy/main/binary-amd64/"
  "jammy-arm.html https://repo.manticoresearch.com/repository/manticoresearch_jammy_dev/dists/jammy/main/binary-arm64/"
  "centos-7-x86_64.html https://repo.manticoresearch.com/repository/manticoresearch/dev/centos/7/x86_64/"
  "centos-7-aarch64.html https://repo.manticoresearch.com/repository/manticoresearch/dev/centos/7/aarch64/"
  "centos-8-x86_64.html https://repo.manticoresearch.com/repository/manticoresearch/dev/centos/8/x86_64/"
  "centos-8-aarch64.html https://repo.manticoresearch.com/repository/manticoresearch/dev/centos/8/aarch64/"
  "centos-9-x86_64.html https://repo.manticoresearch.com/repository/manticoresearch/dev/centos/9/x86_64/"
  "centos-9-aarch64.html https://repo.manticoresearch.com/repository/manticoresearch/dev/centos/9/aarch64/"
  "macos.html https://repo.manticoresearch.com/repository/manticoresearch_macos/dev/"
  "windows-x64.html https://repo.manticoresearch.com/repository/manticoresearch_windows/dev/x64/"
)

for pair in "${urls[@]}"; do
  name="${pair%% *}"
  url="${pair#* }"
  echo "Downloading $name..."
  wget -q -O "$TEMP_DIR/$name" "$url" || { echo "Error: Failed to download $name (URL: $url)"; exit 1; }
done

echo "HTML pages downloaded to $TEMP_DIR."

DEPS_FILE="$REPO_ROOT/deps.txt"
if [ ! -f "$DEPS_FILE" ]; then
  echo "Error: File $DEPS_FILE not found!" >&2
  exit 1
fi

missing_packages=()

while IFS=" " read -r package version_string date hash suffix || [ -n "$package" ]; do
  [[ -z "$package" ]] && continue
  [[ "$package" =~ ^[#[:space:]-] ]] && continue
  [[ "$package" =~ ^[a-z]+$ ]] || continue

  echo "Processing dependency: $package $version_string $date $hash $suffix"

  real_package=$(map_package_name "$package")

  if [[ "$version_string" == *"+"* ]]; then
    version="${version_string%%+*}"
    commit="${version_string#*+}"
    search_substring="${commit%-dev}"
    search_substring="${search_substring##*-}"
    version_format="plus"
  else
    version="$version_string"
    search_substring="$hash"
    version_format="dash"
  fi

  version_escaped=$(echo "$version" | sed 's/\./\\./g')

  if [ "$version_format" = "plus" ]; then
    search_pattern="${real_package}[-_]${version_escaped}(\\+|%2B)[0-9]+[-.]${search_substring}([-.]dev)?([-._a-z0-9]+)?\\.(deb|rpm|tar\\.gz|zip)"
  else
    search_pattern="${real_package}[-_]${version_escaped}[-_][0-9]+[-.]${search_substring}([-.]dev)?([-._a-z0-9]+)?\\.(deb|rpm|tar\\.gz|zip)"
  fi

  found=false

  for html_file in "$TEMP_DIR"/*.html; do
    if grep -E "$search_pattern" "$html_file" >/dev/null 2>&1; then
      found=true
      break
    fi
  done

  if [ "$found" = true ]; then
    echo "✅ Package $package (real name: $real_package) found."
  else
    echo "❌ Package $package (real name: $real_package) NOT found!"
    missing_packages+=("$package $version $search_substring")
  fi
done < "$DEPS_FILE"

rm -rf "$TEMP_DIR"
echo "Temporary HTML files cleaned."

if [ ${#missing_packages[@]} -gt 0 ]; then
  echo ""
  echo "‼️ Error: Some packages are missing:"
  for m in "${missing_packages[@]}"; do
    echo "  - $m"
  done
  exit 1
fi

echo ""
echo "🎉 All packages from deps.txt were found successfully!"
