#!/usr/bin/env bash
#
# Helper for running the Manticore Manual app inside Docker while
# mounting the `manual` directory of the repo as its data source.
# The user must already have access to the Manticore Manual Git repo.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Script is supposed to be in ./misc, so we treat its parent as the repo root.
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DEFAULT_PORT="8080"

IMAGE_NAME="${IMAGE_NAME:-manticore-manual-app:local}"
CONTAINER_NAME="${CONTAINER_NAME:-manticore-manual-app}"
HOST_PORT="${HOST_PORT:-$DEFAULT_PORT}"
MANUAL_SRC="$REPO_ROOT/manual"
DOC_APP_DIR="$REPO_ROOT/misc/doc-app-dir"
DOC_APP_REPO_URL="https://github.com/manticoresoftware/doc.git"
FORCE_NO_CACHE_BUILD=0
error() {
  printf '[error] %s\n' "$*" >&2
  exit 1
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -p|--port)
      HOST_PORT="$2"
      shift 2
      ;;
    -h|--help)
      cat <<'EOF'
Usage: manual.sh [options]

Automates building and running the Manticore Manual Docker image while mounting
the `manual` directory of the repo as /var/manual inside the container.

Options:
  -p, --port                Host port to expose for the application (default: 8080)
  -h, --help                Show this message

EOF
      exit 0
      ;;
    *)
      error "Unknown option: $1"
      ;;
  esac
done

run_docker() {
  if ! command -v docker >/dev/null 2>&1; then
    error "Command 'docker' is required but not found. Please install Docker and try again."
  fi
  if docker "$@" 2>/dev/null; then
    return 0
  fi
  if command -v sudo >/dev/null 2>&1; then
    echo "Docker command failed, retrying with sudo..."
    sudo docker "$@"
  else
    error "Docker command failed."
  fi
}

if [[ ! -d "$MANUAL_SRC" ]]; then
  error "Manual directory '$MANUAL_SRC' does not exist"
fi
MANUAL_SRC="$(cd "$MANUAL_SRC" && pwd)"

if run_docker ps -a --format '{{.Names}}' | grep -Fxq "$CONTAINER_NAME"; then
  error "Container '$CONTAINER_NAME' already exists; stop/remove it before rerunning this script."
fi

if [[ ! -d "$DOC_APP_DIR/.git" ]]; then
  if ! command -v "git" >/dev/null 2>&1; then
    error "Command 'git' is required but not found. Please install Git and try again."
  fi
  rm -rf "$DOC_APP_DIR"
  git clone "$DOC_APP_REPO_URL" "$DOC_APP_DIR"
  FORCE_NO_CACHE_BUILD=1
else
  if ! command -v "git" >/dev/null 2>&1; then
    error "Command 'git' is required but not found. Please install Git and try again."
  fi
  CURRENT_HEAD="$(git -C "$DOC_APP_DIR" rev-parse HEAD)"
  git -C "$DOC_APP_DIR" pull --ff-only
  UPDATED_HEAD="$(git -C "$DOC_APP_DIR" rev-parse HEAD)"
  if [[ "$CURRENT_HEAD" != "$UPDATED_HEAD" ]]; then
    FORCE_NO_CACHE_BUILD=1
  fi
fi

if [[ ! -f "$DOC_APP_DIR/Dockerfile" ]]; then
  error "No Dockerfile found in '$DOC_APP_DIR'; provide the doc app repo root"
fi

if [[ "$FORCE_NO_CACHE_BUILD" -eq 1 ]]; then
  run_docker build --no-cache -t "$IMAGE_NAME" --build-arg DEPLOY_TARGET=local "$DOC_APP_DIR"
else
  run_docker build -t "$IMAGE_NAME" --build-arg DEPLOY_TARGET=local "$DOC_APP_DIR"
fi

printf "Starting container '$CONTAINER_NAME' on port $HOST_PORT..."
run_docker run --rm -d \
  --name "$CONTAINER_NAME" \
  -p "$HOST_PORT":80 \
  -v "$MANUAL_SRC":/var/manual \
  "$IMAGE_NAME" >/dev/null

ready=0
for _ in {1..30}; do
  if curl -s --connect-timeout 2 --max-time 2 "http://localhost:${HOST_PORT}" >/dev/null 2>&1; then
    ready=1
    printf "\n"
    break
  fi
  sleep 1
  printf "."
done
printf "\n"

if [[ "$ready" -eq 0 ]]; then
  echo "Warning: timed out waiting for http://localhost:${HOST_PORT} to respond."
fi

cat <<EOF

Manticore Manual app is running on http://localhost:${HOST_PORT}

To rerun the setup inside the container after editing docs:
  docker exec ${CONTAINER_NAME} ./setup.sh

To validate doc examples/links:
  docker exec ${CONTAINER_NAME} ./setup.sh -d . checkDocs

EOF
