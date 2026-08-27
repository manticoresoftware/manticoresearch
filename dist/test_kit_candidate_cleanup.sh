#!/usr/bin/env bash
set -euo pipefail

: "${REPO_OWNER:?REPO_OWNER is required}"
: "${GH_TOKEN:?GH_TOKEN is required}"

image="${1:?candidate image is required}"
tag="${image##*:}"
case "$tag" in
  test-kit-ci-*) ;;
  *) echo "Refusing to delete non-candidate image tag: $tag" >&2; exit 2 ;;
esac

versions="$(gh api --method GET "/orgs/${REPO_OWNER}/packages/container/manticoresearch/versions?per_page=100")"
matching_versions="$(jq -c --arg tag "$tag" '[.[] | select(.metadata.container.tags | index($tag))]' <<< "$versions")"
match_count="$(jq 'length' <<< "$matching_versions")"

if [[ "$match_count" == "0" ]]; then
  echo "Candidate image tag is already absent: $image"
  exit 0
fi

if [[ "$match_count" != "1" ]]; then
  echo "Candidate image tag maps to multiple package versions: $tag" >&2
  exit 1
fi

if ! jq -e --arg tag "$tag" '.[0].metadata.container.tags == [$tag]' <<< "$matching_versions" >/dev/null; then
  echo "Refusing to delete a package version shared with another tag: $tag" >&2
  exit 1
fi

version_id="$(jq -r '.[0].id' <<< "$matching_versions")"
gh api --method DELETE "/orgs/${REPO_OWNER}/packages/container/manticoresearch/versions/${version_id}"
echo "Deleted temporary CLT candidate image: $image"
