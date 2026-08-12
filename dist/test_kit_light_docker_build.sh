#!/bin/bash
set -euo pipefail

bundle_dir="${1:?Usage: $0 <bundle-package-directory>}"
: "${IMAGE_COMMIT:?IMAGE_COMMIT must be set}"
: "${IMAGE_BRANCH:?IMAGE_BRANCH must be set}"

docker_repo="${DOCKER_REPO:-https://github.com/manticoresoftware/docker.git}"
docker_ref="${DOCKER_REF:-6132e279e57e1ebe226e90d27ee2ba9676a19251}"
http_port="${HTTP_PORT:-18080}"
expected_commit="${EXPECTED_COMMIT:-}"
expected_commit="${expected_commit:0:7}"

work_dir="$(mktemp -d)"
container="manticore-light-smoke-$$"
server_pid=""

cleanup() {
  docker rm -f "$container" >/dev/null 2>&1 || true
  if [[ -n "$server_pid" ]]; then
    kill "$server_pid" >/dev/null 2>&1 || true
    wait "$server_pid" 2>/dev/null || true
  fi
  rm -rf "$work_dir"
}
trap cleanup EXIT

bundle_packages=()
while IFS= read -r package; do
  bundle_packages+=("$package")
done < <(find "$bundle_dir" -type f -name 'manticore_*_amd64.deb' | sort)

if [[ "${#bundle_packages[@]}" -ne 1 ]]; then
  echo "Expected exactly one amd64 Manticore bundle package, found ${#bundle_packages[@]}:" >&2
  find "$bundle_dir" -type f -name 'manticore*.deb' -print | sort >&2 || true
  exit 1
fi

bundle_package="${bundle_packages[0]}"
if ! dpkg-deb -c "$bundle_package" | grep -E '/usr/bin/searchd$' >/dev/null; then
  echo "Package is not a self-contained Manticore bundle: $bundle_package" >&2
  exit 1
fi

http_root="$work_dir/http"
docker_context="$work_dir/docker"
mkdir -p "$http_root"
cp "$bundle_package" "$http_root/manticore_test-kit_amd64.deb"

git init --quiet "$docker_context"
git -C "$docker_context" remote add origin "$docker_repo"
git -C "$docker_context" fetch --quiet --depth 1 origin "$docker_ref"
git -C "$docker_context" checkout --quiet --detach FETCH_HEAD
resolved_docker_ref="$(git -C "$docker_context" rev-parse HEAD)"
if [[ "$docker_ref" =~ ^[0-9a-f]{40}$ && "$resolved_docker_ref" != "$docker_ref" ]]; then
  echo "Expected Docker repository commit $docker_ref, got $resolved_docker_ref" >&2
  exit 1
fi
echo "Using official Docker repository commit $resolved_docker_ref"
test -f "$docker_context/Dockerfile"

python3 -m http.server "$http_port" --bind 0.0.0.0 --directory "$http_root" >"$work_dir/http.log" 2>&1 &
server_pid=$!
for _ in $(seq 1 30); do
  if curl --fail --silent "http://127.0.0.1:${http_port}/manticore_test-kit_amd64.deb" >/dev/null; then
    break
  fi
  sleep 1
done
if ! curl --fail --silent "http://127.0.0.1:${http_port}/manticore_test-kit_amd64.deb" >/dev/null; then
  cat "$work_dir/http.log" >&2
  exit 1
fi

daemon_url="http://host.docker.internal:${http_port}/manticore_test-kit__ARCH_64.deb"
docker build \
  --platform linux/amd64 \
  --add-host host.docker.internal:host-gateway \
  --build-arg DEV=0 \
  --build-arg "DAEMON_URL=$daemon_url" \
  --tag "$IMAGE_COMMIT" \
  --tag "$IMAGE_BRANCH" \
  "$docker_context"

kill "$server_pid" >/dev/null 2>&1 || true
wait "$server_pid" 2>/dev/null || true
server_pid=""

docker run --detach --name "$container" "$IMAGE_COMMIT" >/dev/null
ready=false
for _ in $(seq 1 60); do
  if docker logs "$container" 2>&1 | grep -q 'accepting connections'; then
    ready=true
    break
  fi
  if ! docker inspect --format '{{.State.Running}}' "$container" 2>/dev/null | grep -qx true; then
    break
  fi
  sleep 1
done
if [[ "$ready" != true ]]; then
  docker logs "$container" >&2 || true
  exit 1
fi

version_output="$(docker exec "$container" searchd --version 2>&1)"
printf '%s\n' "$version_output"
if [[ -n "$expected_commit" ]] && ! grep -q "$expected_commit" <<<"$version_output"; then
  echo "Built image version does not contain expected commit $expected_commit" >&2
  exit 1
fi

docker exec "$container" mysql -e "CREATE TABLE light_smoke(title text); INSERT INTO light_smoke(id,title) VALUES(1,'hello'); SELECT id FROM light_smoke WHERE MATCH('hello')" >/dev/null

echo "Built and verified $IMAGE_COMMIT"
echo "Built and verified $IMAGE_BRANCH"
