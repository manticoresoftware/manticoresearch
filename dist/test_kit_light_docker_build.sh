#!/bin/bash
set -euo pipefail

bundle_dir="${1:?Usage: $0 <bundle-package-directory>}"
: "${GITHUB_SHA:?GITHUB_SHA must be set}"

work_dir="$(mktemp -d)"
container="manticore-light-smoke-$$"
server_pid=""
cleanup() {
  docker rm -f "$container" >/dev/null 2>&1 || true
  [[ -z "$server_pid" ]] || { kill "$server_pid" >/dev/null 2>&1 || true; wait "$server_pid" 2>/dev/null || true; }
  rm -rf "$work_dir"
}
trap cleanup EXIT

[[ ! -f "$bundle_dir/artifact.tar" ]] || tar -xf "$bundle_dir/artifact.tar" -C "$bundle_dir"
package="$(find "$bundle_dir" -type f -name 'manticore_[0-9]*_amd64.deb' -print -quit)"
[[ -n "$package" ]] || { echo "Bundle package not found" >&2; exit 1; }

mkdir "$work_dir/http"
cp "$package" "$work_dir/http/manticore_test-kit_amd64.deb"
docker_ref=6132e279e57e1ebe226e90d27ee2ba9676a19251
curl -fsSL "https://github.com/manticoresoftware/docker/archive/$docker_ref.tar.gz" | tar -xz -C "$work_dir"

python3 -m http.server 18080 --bind 0.0.0.0 --directory "$work_dir/http" >"$work_dir/http.log" 2>&1 &
server_pid=$!
docker build --platform linux/amd64 --add-host host.docker.internal:host-gateway \
  --build-arg DEV=0 --build-arg DAEMON_URL=http://host.docker.internal:18080/manticore_test-kit__ARCH_64.deb \
  --tag test-kit-light:img "$work_dir/docker-$docker_ref"

docker run --detach --name "$container" test-kit-light:img >/dev/null
for _ in {1..60}; do
  docker exec "$container" mysql -e 'SHOW VERSION' >/dev/null 2>&1 && break
  sleep 1
done
version="$(docker exec "$container" searchd --version 2>&1)"
printf '%s\n' "$version"
grep -q "${GITHUB_SHA:0:7}" <<<"$version"
docker exec "$container" mysql -e "CREATE TABLE light_smoke(title text); INSERT INTO light_smoke(id,title) VALUES(1,'hello'); SELECT id FROM light_smoke WHERE MATCH('hello')" >/dev/null
docker save test-kit-light:img | gzip -1 > test_kit_light_docker.tar.gz
