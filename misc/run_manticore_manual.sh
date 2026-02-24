#!/usr/bin/env bash
#
# Helper for running the Manticore Manual app inside Docker while
# mounting the `manual` directory of the repo as its data source.
# Note that the user must have access to the Manticore Manual Git repo (github.com/manticoresoftware/doc).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Script is supposed to be located in ./misc, so we treat its parent as the repo root.
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DEFAULT_PORT="8080"
DEFAULT_DOC_APP_DIR="$REPO_ROOT"

IMAGE_NAME="${IMAGE_NAME:-manticore-manual-app:local}"
CONTAINER_NAME="${CONTAINER_NAME:-manticore-manual-app}"
HOST_PORT="${HOST_PORT:-$DEFAULT_PORT}"
MANUAL_SRC="$REPO_ROOT/manual"
DOC_APP_DIR_INPUT="${DOC_APP_DIR:-$DEFAULT_DOC_APP_DIR}"
DOC_APP_REPO_URL="https://github.com/manticoresoftware/doc.git"
WORK_DIR="$REPO_ROOT/.manticore_manual_app"
USE_CACHED_IMAGE=0
USE_SUDO=0


error() {
  printf '[error] %s\n' "$*" >&2
  exit 1
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --doc-app-dir)
      DOC_APP_DIR="$2"
      shift 2
      ;;
    -C|--use-cached-image)
      USE_CACHED_IMAGE=1
      shift
      ;;
    -p|--port)
      HOST_PORT="$2"
      shift 2
      ;;
    -S|--use-sudo)
      USE_SUDO=1
      shift
      ;;
    -h|--help)
      cat <<'EOF'
Usage: run_manticore_manual.sh [options]

Automates building and running the Manticore Manual app in the Docker container
while mounting the `manual` directory of the repo as its data source.

Options:
  --doc-app-dir             Path to a local checkout of the doc app repo; not set by default
  -C, --use-cached-image    Reuse the already built Docker image for the application; disabled by default
  -S, --use-sudo            Run script Docker commands with `sudo` in case it's required on your local machine; disabled by default
  -p, --port                Host port to expose for the application; 8080 by default
  -h, --help                Show this message

EOF
      exit 0
      ;;
    *)
      error "Unknown option: $1"
      ;;
  esac
done

DOCKER_CMD="docker"
if [[ "$USE_SUDO" -eq 1 ]]; then
  DOCKER_CMD="sudo docker"
fi

if ! command -v ${DOCKER_CMD%% *} >/dev/null 2>&1; then
    error "Command 'docker' is required but not found. Please install Docker and try again."
fi

if [[ ! -d "$MANUAL_SRC" ]]; then
  error "Manual directory '$MANUAL_SRC' does not exist"
fi
MANUAL_SRC="$(cd "$MANUAL_SRC" && pwd)"

DOC_APP_DIR=""
if [[ -d "$DOC_APP_DIR_INPUT" ]]; then
  if [[ -f "$DOC_APP_DIR_INPUT/Dockerfile" ]]; then
    DOC_APP_DIR="$(cd "$DOC_APP_DIR_INPUT" && pwd)"
  fi
fi

if [[ -z "$DOC_APP_DIR" ]]; then
  if ! command -v "git" >/dev/null 2>&1; then
    error "Command 'git' is required but not found. Please install Git and try again."
  fi
  if [[ -d "$WORK_DIR/.git" ]]; then
    git -C "$WORK_DIR" pull --ff-only
  elif [[ -d "$WORK_DIR" ]]; then
    error "Path '$WORK_DIR' exists but is not a git checkout; remove it or set DOC_APP_DIR"
  else
    git clone "$DOC_APP_REPO_URL" "$WORK_DIR"
  fi
  DOC_APP_DIR="$WORK_DIR"
fi

if [[ ! -f "$DOC_APP_DIR/Dockerfile" ]]; then
  error "No Dockerfile found in '$DOC_APP_DIR'; provide the doc app repo root"
fi

if [[ "$USE_CACHED_IMAGE" -eq 1 ]]; then
  $DOCKER_CMD build -t "$IMAGE_NAME" --build-arg DEPLOY_TARGET=local "$DOC_APP_DIR"
else
  $DOCKER_CMD build --no-cache -t "$IMAGE_NAME" --build-arg DEPLOY_TARGET=local "$DOC_APP_DIR"
fi

if [[ "$DOC_APP_DIR" == "$WORK_DIR" ]]; then
  rm -rf "$WORK_DIR"
fi

if $DOCKER_CMD ps -a --format '{{.Names}}' | grep -Fxq "$CONTAINER_NAME"; then
  error "Container '$CONTAINER_NAME' already exists; stop/remove it before rerunning"
fi

printf "Starting container '$CONTAINER_NAME' on port $HOST_PORT..."
$DOCKER_CMD run --rm -d \
  --name "$CONTAINER_NAME" \
  -p "$HOST_PORT":80 \
  -v "$MANUAL_SRC":/var/manual \
  "$IMAGE_NAME" >/dev/null

for _ in {1..30}; do
  if curl -fs "http://localhost:${HOST_PORT}" >/dev/null 2>&1; then
    printf "\n"
    break
  fi
  sleep 1
  printf "."
done
printf "\n"

cat <<EOF

Manticore Manual app is running on http://localhost:${HOST_PORT}

To rerun the setup inside the container after editing docs:
  docker exec ${CONTAINER_NAME} ./setup.sh

To validate doc examples/links:
  docker exec ${CONTAINER_NAME} ./setup.sh -d . checkDocs

EOF
