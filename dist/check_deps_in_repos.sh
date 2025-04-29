#!/bin/bash
set -e

SCRIPT_DIR="$(dirname "$0")"
REPO_ROOT="$(realpath "$SCRIPT_DIR/..")"
echo "Current directory: $(pwd)"
echo "Script location: $SCRIPT_DIR"
echo "Expected repository root: $REPO_ROOT"

TEMP_DIR=$(mktemp -d)
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
  wget -q -O "$name" "$url" || { echo "Error: Failed to download $name (URL: $url)"; exit 1; }
done

echo "HTML pages downloaded."

DEPS_FILE="$REPO_ROOT/deps.txt"
if [ ! -f "$DEPS_FILE" ]; then
  echo "File $DEPS_FILE not found!" >&2
  exit 1
fi

missing_packages=()

while IFS=" " read -r package version_string date hash suffix || [ -n "$package" ]; do
  [[ -z "$package" ]] && continue
  [[ "$package" =~ ^[#[:space:]-] ]] && continue
  [[ "$package" =~ ^[a-z]+$ ]] || continue

  echo "Processing dependency: $package $version_string $date $hash $suffix"

  case $package in
    backup) package="manticore-backup" ;;
    buddy) package="manticore-buddy" ;;
    load) package="manticore-load" ;;
    mcl) package="manticore-columnar-lib" ;;
    tzdata) package="manticore-tzdata" ;;
    executor) package="manticore-executor" ;;
    read) package="manticore-read" ;;
    grok) package="manticore-grok" ;;
    *) echo "Unknown package: $package" ; continue ;;
  esac

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
    search_pattern="${package}[-_]${version_escaped}(\\+|%2B)[0-9]+[-.]${search_substring}([-.]dev)?([-._a-z0-9]+)?\\.(deb|rpm|tar\\.gz|zip)"
  else
    search_pattern="${package}[-_]${version_escaped}[-_][0-9]+[-.]${search_substring}([-.]dev)?([-._a-z0-9]+)?\\.(deb|rpm|tar\\.gz|zip)"
  fi

  found_in_repos=()
  missing_in_repos=()

  for html_file in *.html; do
    if grep -E "$search_pattern" "$html_file" >/dev/null 2>&1; then
      found_in_repos+=("$html_file")
    else
      missing_in_repos+=("$html_file")
    fi
  done

  if [ ${#found_in_repos[@]} -gt 0 ]; then
    echo "Package $package found in:"
    for repo in "${found_in_repos[@]}"; do
      echo "  - $repo"
    done
  fi

  if [ ${#missing_in_repos[@]} -gt 0 ]; then
    echo "Package $package missing in:"
    for repo in "${missing_in_repos[@]}"; do
      echo "  - $repo"
    done
  else
    echo "Package $package found in all repositories!"
  fi

  if [ ${#found_in_repos[@]} -eq 0 ]; then
    echo "Package $package not found!"
    missing_packages+=("$package $version $search_substring")
  fi
done < "$DEPS_FILE"

if [ ${#missing_packages[@]} -gt 0 ]; then
  echo "Error: Some packages are missing:"
  for m in "${missing_packages[@]}"; do
    echo "  - $m"
  done
  exit 1
fi

echo "All packages from deps.txt were found successfully!"
rm -rf "$TEMP_DIR"
