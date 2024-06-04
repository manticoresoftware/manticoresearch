#!/bin/bash
set -e

[ -n "$GHCR_USER" ] && echo "$GHCR_PASSWORD" | docker login -u"$GHCR_USER" --password-stdin ghcr.io

if ! (docker info | grep Username) > /dev/null 2>&1; then
  echo "Can't authorise to GHCR docker registry"
  exit 1
fi

# Helper to fix tag naming from branch -> docker tag allowance
sanitize_tag() {
    local name=$1
    # Replace / with _
    name=$(echo "$name" | tr '/' '_')
    # Remove all characters that are not allowed in Docker tags
    # Docker tag rules: https://docs.docker.com/engine/reference/commandline/tag/
    # Tags can only contain lowercase and uppercase letters, digits, underscores, periods, and dashes
    name=$(echo "$name" | sed 's/[^a-zA-Z0-9_.-]//g')
    echo "$name"
}

hub_repo="ghcr.io/${REPO_OWNER}/manticoresearch"
img_url="${hub_repo}:test-kit-${BUILD_COMMIT}"
images=("$img_url")
[[ $GITHUB_REF_NAME == "master" ]] \
  && img_url_latest="test-kit-latest" \
  && images+=("$img_url_latest") \
  || img_url_latest=""

# Get the latest tag from the git references
# Check if any tag exists
latest_tag=$(git describe --abbrev=0 --tags 2>/dev/null || echo "")

# Assign the current branch or tag to the appropriate variable
if [ -n "$latest_tag" ]; then
	img_url_tag="test-kit-$(sanitize_tag "$latest_tag")"
	images+=("$img_url_tag")
fi

# Custom branch name
if [ "$GITHUB_REF_NAME" != "master" ]; then
	img_url_branch="test-kit-$(sanitize_tag "$GITHUB_REF_NAME")"
	images+=("$img_url_branch")
fi

echo "Going to push to '$img_url' and ('$img_url_latest', '$img_url_tag', '$img_url_branch') (if not empty) if there's access to the registry"

# exporting the image, it also squashes all the layers into one
docker import ./manticore_test_kit.img "$img_url"
[ -n "$img_url_latest" ] && docker tag "$img_url" "$img_url_latest"
[ -n "$img_url_tag" ] && docker tag "$img_url" "$img_url_tag"
[ -n "$img_url_branch" ] && docker tag "$img_url" "$img_url_branch"

# pusing to ghcr.io
[ -n "$GHCR_USER" ] && for img in "${images[@]}"; do
	docker push "$img" \
	  && echo "❗ Pushed the image to $img" \
      && echo "Pushed test-kit to $img" >> "$GITHUB_STEP_SUMMARY" \
	  || echo "❗ Couldn't push the image to $img"
done || echo "Skipped pushing to repo, because GHCR_USER is not set"
