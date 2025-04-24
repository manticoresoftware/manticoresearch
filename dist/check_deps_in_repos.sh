#!/bin/bash
set -e

# Calculate the path to the repository root relative to the script location
SCRIPT_DIR="$(dirname "$0")"
REPO_ROOT="$(realpath "$SCRIPT_DIR/..")"
echo "Current directory: $(pwd)"
echo "Script location: $SCRIPT_DIR"
echo "Expected repository root: $REPO_ROOT"

# Create a temporary directory for HTML files
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

# Download all repositories
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
)

for pair in "${urls[@]}"; do
  name="${pair%% *}"
  url="${pair#* }"
  echo "Downloading $name..."
  wget -q -O "$name" "$url" || { echo "Error: Failed to download $name (URL: $url)"; exit 1; }
done

echo "HTML pages downloaded."

# Check for the existence of deps.txt
DEPS_FILE="$REPO_ROOT/deps.txt"
if [ ! -f "$DEPS_FILE" ]; then
  echo "File $DEPS_FILE not found!" >&2
  exit 1
fi

# Array to store missing packages
missing_packages=()

# Process the first 6 lines of deps.txt
while IFS=" " read -r package version_string date hash suffix || [ -n "$package" ]; do
  [[ -z "$package" ]] && continue

  echo "Processing dependency: $package $version_string $date $hash $suffix"

  # Map package names to repository format
  case $package in
    backup) package="manticore-backup" ;;
    buddy) package="manticore-buddy" ;;  # Now checking as a regular package
    load) package="manticore-load" ;;
    mcl) package="manticore-columnar-lib" ;;
    tzdata) package="manticore-tzdata" ;;
    executor) package="manticore-executor" ;;
    *) echo "Unknown package: $package"; continue ;;
  esac

  # Determine version and search substring (hash)
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

  # Escape dots in the version for regex
  version_escaped=$(echo "$version" | sed 's/\./\\./g')

  # Form the search pattern
  if [ "$version_format" = "plus" ]; then
    # For format with "+" (e.g., manticore-backup-1.9.1+25040420-c6e46da2-dev-1.el7.noarch.rpm)
    search_pattern="${package}[-_]${version_escaped}(\\+|%2B)[0-9]+[-.]${search_substring}([-.]dev)?([-._a-z0-9]+)?\\.(deb|rpm|tar\\.gz|zip)"
  else
    # For format with "-" (e.g., manticore-tzdata-1.0.1_240904.3ba592a-1.noarch.rpm)
    search_pattern="${package}[-_]${version_escaped}[-_][0-9]+[-.]${search_substring}([-.]dev)?([-._a-z0-9]+)?\\.(deb|rpm|tar\\.gz|zip)"
  fi

  # Search for the package in all HTML files
  found_in_repos=()
  missing_in_repos=()

  for html_file in *.html; do
    if grep -E "$search_pattern" "$html_file" >/dev/null 2>&1; then
      found_in_repos+=("$html_file")
    else
      missing_in_repos+=("$html_file")
    fi
  done

  # Output results
  if [ ${#found_in_repos[@]} -gt 0 ]; then
    echo "Package $package with version $version and substring $search_substring found in:"
    for repo in "${found_in_repos[@]}"; do
      echo "  - $repo"
    done
  fi

  if [ ${#missing_in_repos[@]} -gt 0 ]; then
    echo "Package $package with version $version and substring $search_substring not found in:"
    for repo in "${missing_in_repos[@]}"; do
      echo "  - $repo"
    done
  else
    echo "Package $package with version $version and substring $search_substring found in all repositories!"
  fi

  # If package not found in any repository, add to missing_packages
  if [ ${#found_in_repos[@]} -eq 0 ]; then
    echo "Package $package with version $version and substring $search_substring not found."
    missing_packages+=("$package $version $search_substring")
  fi
done < <(head -n 6 "$DEPS_FILE")

# Check if there are missing packages
if [ ${#missing_packages[@]} -gt 0 ]; then
  echo "Error: The following packages were not found:"
  for m in "${missing_packages[@]}"; do
    echo "  - $m"
  done
  exit 1
fi

echo "All packages from deps.txt were found in the repositories!"

# Clean up temporary HTML files
rm -rf "$TEMP_DIR"
