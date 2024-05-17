#!/bin/bash
set -e

[ -n "$GHCR_USER" ] && echo "$GHCR_PASSWORD" | docker login -u"$GHCR_USER" --password-stdin ghcr.io

if ! (docker info | grep Username) > /dev/null 2>&1; then
  echo "Can't authorise to GHCR docker registry"
  exit 1
fi

img_url="ghcr.io/${REPO_OWNER}/manticoresearch:test-kit-${BUILD_COMMIT}"
images=("$img_url")
[[ $GITHUB_REF_NAME == "master" ]] \
  && img_url_latest="ghcr.io/${REPO_OWNER}/manticoresearch:test-kit-latest" \
  && images+=("$img_url_latest") \
  || img_url_latest=""

echo "Going to push to '$img_url' and '$img_url_latest' (if not empty) if there's access to the registry"

# exporting the image, it also squashes all the layers into one
docker import ./manticore_test_kit.img "$img_url"
[ -n "$img_url_latest" ] && docker tag "$img_url" "$img_url_latest"

# pusing to ghcr.io
[ -n "$GHCR_USER" ] && for img in "${images[@]}"; do
	docker push "$img" \
	  && echo "❗ Pushed the image to $img" \
      && echo "Pushed test-kit to $img" >> "$GITHUB_STEP_SUMMARY" \
	  || echo "❗ Couldn't push the image to $img"
done || echo "Skipped pushing to repo, because GHCR_USER is not set"
