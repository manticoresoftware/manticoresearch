#!/bin/bash
set -e

SCRIPT_DIR="$(dirname "$0")"
REPO_ROOT="$(realpath "$SCRIPT_DIR/..")"
TEMP_DIR=$(mktemp -d)

echo "Current directory: $(pwd)"
echo "Script location: $SCRIPT_DIR"
echo "Expected repository root: $REPO_ROOT"
echo "Temporary directory: $TEMP_DIR"

# List of HTML pages to download
declare -a urls=(
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
  "centos-8-x86_64.html https://repo.manticoresearch.com/repository/manticoresearch/dev/centos/8/x86_64/"
  "centos-8-aarch64.html https://repo.manticoresearch.com/repository/manticoresearch/dev/centos/8/aarch64/"
  "centos-9-x86_64.html https://repo.manticoresearch.com/repository/manticoresearch/dev/centos/9/x86_64/"
  "centos-9-aarch64.html https://repo.manticoresearch.com/repository/manticoresearch/dev/centos/9/aarch64/"
  "macos.html https://repo.manticoresearch.com/repository/manticoresearch_macos/dev/"
  "windows-x64.html https://repo.manticoresearch.com/repository/manticoresearch_windows/dev/x64/"
)

# List of exceptions: allowed missing packages in specific repositories
# Format: "package:repository"
SKIP_CHECKS=(
  "executor:windows-x64.html"
  "tzdata:macos.html"
)

should_skip() {
  local pkg="$1"
  local repo="$2"
  for entry in "${SKIP_CHECKS[@]}"; do
    if [[ "$entry" == "${pkg}:${repo}" ]]; then
      return 0
    fi
  done
  return 1
}

# Download HTML files into TEMP_DIR
pids=()
for pair in "${urls[@]}"; do
  name="${pair%% *}"
  url="${pair#* }"
  echo "Starting download for $name..."
  wget -q -O "$TEMP_DIR/$name" "$url" &
  pids+=($!)
done

# Wait for all download processes to complete
echo "Waiting for all downloads to complete..."
for pid in "${pids[@]}"; do
  wait $pid || { echo "Error: One of the downloads failed"; exit 1; }
done
echo "All HTML pages were successfully downloaded to $TEMP_DIR."

# Read deps.txt file
DEPS_FILE="$REPO_ROOT/deps.txt"
if [ ! -f "$DEPS_FILE" ]; then
  echo "Error: deps.txt not found!" >&2
  exit 1
fi

missing_packages=()

while IFS=" " read -r package version_string date hash suffix || [ -n "$package" ]; do
  [[ -z "$package" ]] && continue
  [[ "$package" =~ ^[#[:space:]-] ]] && continue
  [[ "$package" =~ ^[a-z]+$ ]] || continue

  case $package in
    backup) real_name="manticore-backup" ;;
    buddy) real_name="manticore-buddy" ;;
    load) real_name="manticore-load" ;;
    mcl) real_name="manticore-columnar-lib" ;;
    tzdata) real_name="manticore-tzdata" ;;
    executor) real_name="manticore-executor" ;;
    *) real_name="$package" ;;
  esac

  echo "Processing dependency: $package (real name: $real_name) $version_string"

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
    search_pattern="${real_name}[-_]${version_escaped}(\\+|%2B)[0-9]+[-.]${search_substring}([-.]dev)?([-._a-z0-9]+)?\\.(deb|rpm|tar\\.gz|zip)"
  else
    search_pattern="${real_name}[-_]${version_escaped}[-_][0-9]+[-.]${search_substring}([-.]dev)?([-._a-z0-9]+)?\\.(deb|rpm|tar\\.gz|zip)"
  fi

  missing_in=()

  for html_file in "$TEMP_DIR"/*.html; do
    repo_name="$(basename "$html_file")"
    if should_skip "$package" "$repo_name"; then
      echo "ℹ️  Skipping check for $package in $repo_name"
      continue
    fi
    if ! grep -E "$search_pattern" "$html_file" >/dev/null 2>&1; then
      missing_in+=("$repo_name")
    fi
  done

  if [ ${#missing_in[@]} -gt 0 ]; then
    echo "❌ Package $package (real name: $real_name) is missing in:"
    for repo in "${missing_in[@]}"; do
      echo "  - $repo"
    done
    missing_packages+=("$package $version $search_substring")
  else
    echo "✅ Package $package (real name: $real_name) was found in all required repositories."
  fi
done < "$DEPS_FILE"

# Final summary
if [ ${#missing_packages[@]} -gt 0 ]; then
  echo ""
  echo "‼️ Error: The following packages are missing:"
  for m in "${missing_packages[@]}"; do
    echo "  - $m"
  done
  rm -rf "$TEMP_DIR"
  exit 1
else
  echo ""
  echo "🎉 All packages listed in deps.txt were successfully found!"
  rm -rf "$TEMP_DIR"
fi
