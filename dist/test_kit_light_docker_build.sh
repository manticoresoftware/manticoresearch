#!/bin/bash
set -euo pipefail

bundle_dir="${1:?Usage: $0 <bundle-package-directory>}"

work_dir="$(mktemp -d)"
cleanup() {
  rm -rf "$work_dir"
}
trap cleanup EXIT

[[ ! -f "$bundle_dir/artifact.tar" ]] || tar -xf "$bundle_dir/artifact.tar" -C "$bundle_dir"
package="$(find "$bundle_dir" -type f -name 'manticore_[0-9]*_amd64.deb' -print -quit)"
[[ -n "$package" ]] || { echo "Bundle package not found" >&2; exit 1; }

docker_ref=6132e279e57e1ebe226e90d27ee2ba9676a19251
curl -fsSL "https://github.com/manticoresoftware/docker/archive/$docker_ref.tar.gz" | tar -xz -C "$work_dir"
docker_dir="$work_dir/docker-$docker_ref"
cp "$package" "$docker_dir/manticore_test-kit_amd64.deb"
sed -i.bak "s/^#ADD \\*deb /ADD *deb /; s/^ENV DAEMON_URL .*/ENV DAEMON_URL \${DAEMON_URL}/" "$docker_dir/Dockerfile"

# The Dockerfile fetches the gosu signing key from a single key server, so the image can't be built while that server
# is down. Until the docker repo handles it, fall back to a second server that holds the same key.
# Remove this once $docker_ref points at a Dockerfile that no longer depends on one key server.
recv_keys='gpg --batch --keyserver hkps://keys.openpgp.org --recv-keys'
grep -q -- "$recv_keys" "$docker_dir/Dockerfile" \
  || { echo "Key fetch command not found in the Dockerfile; the key server fallback needs updating" >&2; exit 1; }
sed -i.bak "s#$recv_keys \\([0-9A-Fa-f]*\\)#{ $recv_keys \\1 || gpg --batch --keyserver hkps://keyserver.ubuntu.com --recv-keys \\1; }#" "$docker_dir/Dockerfile"
rm "$docker_dir/Dockerfile.bak"

docker build --platform linux/amd64 --build-arg DEV=0 --build-arg DAEMON_URL= \
  --tag test-kit-light:img "$docker_dir"

docker save test-kit-light:img | gzip -1 > test_kit_light_docker.tar.gz
