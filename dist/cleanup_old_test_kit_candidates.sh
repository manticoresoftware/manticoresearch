#!/usr/bin/env bash
set -euo pipefail

: "${REPO_OWNER:?REPO_OWNER is required}"
: "${GH_TOKEN:?GH_TOKEN is required}"

cutoff="$(date -u -d '7 days ago' +%s)"
versions="$(gh api --paginate --method GET "/orgs/${REPO_OWNER}/packages/container/manticoresearch/versions?per_page=100" | jq -cs 'add')"

candidates="$(jq -c --argjson cutoff "$cutoff" '
  .[]
  | select((.metadata.container.tags | length) == 1)
  | .metadata.container.tags[0] as $tag
  | select($tag | test("^test-kit-ci-[0-9a-f]{40}$"))
  | select((.created_at | fromdateiso8601) < $cutoff)
  | {id, tag: $tag, created_at}
' <<< "$versions")"

if [[ -z "$candidates" ]]; then
  echo "No temporary CLT candidate images older than seven days"
  exit 0
fi

deleted=0
while IFS= read -r candidate; do
  version_id="$(jq -r '.id' <<< "$candidate")"
  tag="$(jq -r '.tag' <<< "$candidate")"
  created_at="$(jq -r '.created_at' <<< "$candidate")"
  gh api --method DELETE "/orgs/${REPO_OWNER}/packages/container/manticoresearch/versions/${version_id}"
  printf 'Deleted temporary CLT candidate image %s created at %s\n' "$tag" "$created_at"
  ((deleted += 1))
done <<< "$candidates"

printf 'Deleted %s temporary CLT candidate image version(s) older than seven days\n' "$deleted"