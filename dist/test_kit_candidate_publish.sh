#!/usr/bin/env bash
set -euo pipefail

: "${GHCR_USER:?GHCR_USER is required}"
: "${GHCR_PASSWORD:?GHCR_PASSWORD is required}"
: "${REPO_OWNER:?REPO_OWNER is required}"
: "${GITHUB_RUN_ID:?GITHUB_RUN_ID is required}"
: "${GITHUB_SHA:?GITHUB_SHA is required}"
: "${GITHUB_OUTPUT:?GITHUB_OUTPUT is required}"

image="ghcr.io/${REPO_OWNER}/manticoresearch:test-kit-ci-${GITHUB_RUN_ID}-${GITHUB_SHA}"

echo "$GHCR_PASSWORD" | docker login -u"$GHCR_USER" --password-stdin ghcr.io
docker import \
  --change "LABEL org.manticore.testkit.candidate=true" \
  --change "LABEL org.manticore.testkit.run_id=${GITHUB_RUN_ID}" \
  --change "LABEL org.manticore.testkit.commit=${GITHUB_SHA}" \
  manticore_test_kit.img "$image"
docker push "$image"

echo "candidate_image=$image" >> "$GITHUB_OUTPUT"
echo "Published temporary CLT candidate image: $image" >> "$GITHUB_STEP_SUMMARY"
