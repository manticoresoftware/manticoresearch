#!/bin/bash
set -euo pipefail

# Parse command line arguments
REPO_TYPE="dev"  # Default value
while [[ $# -gt 0 ]]; do
  case $1 in
    --target)
      if [[ "$2" == "release" ]]; then
        REPO_TYPE="release_candidate"
      fi
      shift 2
      ;;
    *)
      echo "Unknown option: $1"
      echo "Usage: $0 [--target release]"
      exit 1
      ;;
  esac
done

SCRIPT_DIR="$(dirname "$0")"
REPO_ROOT="$(realpath "$SCRIPT_DIR/..")"
TEMP_DIR=$(mktemp -d)
ARTIFACT_DIR="$REPO_ROOT/check_deps_debug_artifact"
PRESERVE_DEBUG=0

echo "Current directory: $(pwd)"
echo "Script location: $SCRIPT_DIR"
echo "Expected repository root: $REPO_ROOT"
echo "Temporary directory: $TEMP_DIR"
echo "Using repository type: $REPO_TYPE"

cleanup() {
  if [ "$PRESERVE_DEBUG" -eq 0 ] && [ -d "$TEMP_DIR" ]; then
    rm -rf "$TEMP_DIR"
  fi
}

trap cleanup EXIT

prepare_debug_bundle() {
  local reason="$1"
  PRESERVE_DEBUG=1
  rm -rf "$ARTIFACT_DIR"
  mkdir -p "$ARTIFACT_DIR"
  cp -R "$TEMP_DIR"/. "$ARTIFACT_DIR"/
  {
    echo "reason=$reason"
    echo "repo_type=$REPO_TYPE"
    echo "cwd=$(pwd)"
    echo "script_dir=$SCRIPT_DIR"
    echo "repo_root=$REPO_ROOT"
    echo "temp_dir=$TEMP_DIR"
    echo "artifact_dir=$ARTIFACT_DIR"
  } > "$ARTIFACT_DIR/context.txt"
}

# List of HTML pages to download
declare -a urls=(
  "bionic-amd.html https://repo.manticoresearch.com/repository/manticoresearch_bionic_${REPO_TYPE}/dists/bionic/main/binary-amd64/"
  "bionic-arm.html https://repo.manticoresearch.com/repository/manticoresearch_bionic_${REPO_TYPE}/dists/bionic/main/binary-arm64/"
  "bookworm-amd.html https://repo.manticoresearch.com/repository/manticoresearch_bookworm_${REPO_TYPE}/dists/bookworm/main/binary-amd64/"
  "bookworm-arm.html https://repo.manticoresearch.com/repository/manticoresearch_bookworm_${REPO_TYPE}/dists/bookworm/main/binary-arm64/"
  "bullseye-amd.html https://repo.manticoresearch.com/repository/manticoresearch_bullseye_${REPO_TYPE}/dists/bullseye/main/binary-amd64/"
  "bullseye-arm.html https://repo.manticoresearch.com/repository/manticoresearch_bullseye_${REPO_TYPE}/dists/bullseye/main/binary-arm64/"
  "buster-amd.html https://repo.manticoresearch.com/repository/manticoresearch_buster_${REPO_TYPE}/dists/buster/main/binary-amd64/"
  "buster-arm.html https://repo.manticoresearch.com/repository/manticoresearch_buster_${REPO_TYPE}/dists/buster/main/binary-arm64/"
  "focal-amd.html https://repo.manticoresearch.com/repository/manticoresearch_focal_${REPO_TYPE}/dists/focal/main/binary-amd64/"
  "focal-arm.html https://repo.manticoresearch.com/repository/manticoresearch_focal_${REPO_TYPE}/dists/focal/main/binary-arm64/"
  "jammy-amd.html https://repo.manticoresearch.com/repository/manticoresearch_jammy_${REPO_TYPE}/dists/jammy/main/binary-amd64/"
  "jammy-arm.html https://repo.manticoresearch.com/repository/manticoresearch_jammy_${REPO_TYPE}/dists/jammy/main/binary-arm64/"
  "centos-8-x86_64.html https://repo.manticoresearch.com/repository/manticoresearch/${REPO_TYPE}/centos/8/x86_64/"
  "centos-8-aarch64.html https://repo.manticoresearch.com/repository/manticoresearch/${REPO_TYPE}/centos/8/aarch64/"
  "centos-9-x86_64.html https://repo.manticoresearch.com/repository/manticoresearch/${REPO_TYPE}/centos/9/x86_64/"
  "centos-9-aarch64.html https://repo.manticoresearch.com/repository/manticoresearch/${REPO_TYPE}/centos/9/aarch64/"
  "centos-10-x86_64.html https://repo.manticoresearch.com/repository/manticoresearch/${REPO_TYPE}/centos/10/x86_64/"
  "centos-10-aarch64.html https://repo.manticoresearch.com/repository/manticoresearch/${REPO_TYPE}/centos/10/aarch64/"
  "macos.html https://repo.manticoresearch.com/repository/manticoresearch_macos/${REPO_TYPE}/"
  "windows-x64.html https://repo.manticoresearch.com/repository/manticoresearch_windows/${REPO_TYPE}/x64/"
)

# List of exceptions: allowed missing packages in specific repositories
# Format: "package:repository"
SKIP_CHECKS=(
  "executor:windows-x64.html"
  "tzdata:macos.html"
  "galera:windows-x64.html"
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
mkdir -p "$TEMP_DIR/wget_logs"
pids=()
for pair in "${urls[@]}"; do
  name="${pair%% *}"
  url="${pair#* }"
  echo "Starting download for $name..."
  wget -c --tries=10 --retry-connrefused --waitretry=5 --timeout=60 --server-response --output-file="$TEMP_DIR/wget_logs/${name}.log" -O "$TEMP_DIR/$name" "$url" >/dev/null 2>&1 &
  pids+=($!)
done

# Wait for all download processes to complete
echo "Waiting for all downloads to complete..."
for pid in "${pids[@]}"; do
  if ! wait "$pid"; then
    echo "Error: One of the downloads failed"
    prepare_debug_bundle "download_failed"
    echo "Debug artifact prepared in $ARTIFACT_DIR"
    exit 1
  fi
done
echo "All HTML pages were successfully downloaded to $TEMP_DIR."

download_summary="$TEMP_DIR/download_summary.txt"
printf "repo\tbytes\tstatus\tcontent_type\tpackage_links\ttitle\n" > "$download_summary"

invalid_downloads=()
for html_file in "$TEMP_DIR"/*.html; do
  repo_name="$(basename "$html_file")"
  log_file="$TEMP_DIR/wget_logs/${repo_name}.log"
  size_bytes=$(wc -c < "$html_file" | tr -d ' ')
  status_code=$( (grep -Eo 'HTTP/[0-9.]+ [0-9]+' "$log_file" || true) | tail -1 | awk '{print $2}' )
  content_type=$( (grep -i '^  Content-Type:' "$log_file" || true) | tail -1 | sed 's/^  Content-Type: //; s/\r$//' )
  package_links=$( (grep -Eio '\.(deb|rpm|tar\.gz|zip)(\"|<|$)' "$html_file" || true) | wc -l | tr -d ' ' )
  title=$( { tr '\n' ' ' < "$html_file" | sed -n 's:.*<title>\(.*\)</title>.*:\1:Ip' | sed 's/[[:space:]]\+/ /g; s/^ //; s/ $//'; } || true )

  printf "%s\t%s\t%s\t%s\t%s\t%s\n" \
    "$repo_name" "${size_bytes:-0}" "${status_code:-unknown}" "${content_type:-unknown}" "${package_links:-0}" "${title:-n/a}" \
    >> "$download_summary"

  if [ "${size_bytes:-0}" -eq 0 ] || [ "${package_links:-0}" -eq 0 ]; then
    invalid_downloads+=("$repo_name|size=${size_bytes:-0}|status=${status_code:-unknown}|content_type=${content_type:-unknown}|links=${package_links:-0}|title=${title:-n/a}")
  fi
done

if [ ${#invalid_downloads[@]} -gt 0 ]; then
  echo "Warning: Some downloaded pages do not look like package indexes."
  for entry in "${invalid_downloads[@]}"; do
    IFS='|' read -r repo_name size_info status_info content_type_info links_info title_info <<< "$entry"
    echo "  - $repo_name ($size_info, $status_info, $content_type_info, $links_info, $title_info)"
  done
fi

# Read deps.txt file
DEPS_FILE="$REPO_ROOT/deps.txt"
if [ ! -f "$DEPS_FILE" ]; then
  echo "Error: deps.txt not found!" >&2
  exit 1
fi

missing_packages=()
declare -A repo_issue_details=()

if [ ${#invalid_downloads[@]} -gt 0 ]; then
  for entry in "${invalid_downloads[@]}"; do
    IFS='|' read -r repo_name size_info status_info content_type_info links_info title_info <<< "$entry"
    repo_issue_details["$repo_name"]="$size_info, $status_info, $content_type_info, $links_info, $title_info"
  done
fi

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
    galera) real_name="manticore-galera" ;;
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
  elif [ -z "$search_substring" ]; then
    # Simple version only (e.g. galera 3.37): match _3.37_amd64.deb, -3.37-1.el8.x86_64.rpm, -3.37-Darwin-osx11.6-arm64.tar.gz
    search_pattern="${real_name}[-_]${version_escaped}([-_][-a-zA-Z0-9._]+)?\\.(deb|rpm|tar\\.gz|zip)"
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
      extra_info="${repo_issue_details[$repo]:-}"
      if [ -n "$extra_info" ]; then
        echo "  - $repo [$extra_info]"
      else
        echo "  - $repo"
      fi
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
  prepare_debug_bundle "packages_missing_or_unparseable_pages"
  cp "$download_summary" "$ARTIFACT_DIR/download_summary.txt"
  echo "Debug artifact prepared in $ARTIFACT_DIR"
  exit 1
else
  echo ""
  echo "🎉 All packages listed in deps.txt were successfully found!"
fi
