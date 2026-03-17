#!/usr/bin/env bash
#
# Helper for running the Manticore Manual app inside Docker while
# mounting the `manual` directory of the repo as its data source.
# The user must already have access to the Manticore Manual Git repo.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Script is supposed to be in ./misc, so we treat its parent as the repo root.
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

ENV_FILE="$REPO_ROOT/.env"
if [[ -f "$ENV_FILE" ]]; then
  set -a
  set +u
  # shellcheck disable=SC1090
  source "$ENV_FILE"
  set -u
  set +a
fi

DEFAULT_PORT="8080"

IMAGE_NAME="${IMAGE_NAME:-manticore-manual-app:local}"
CONTAINER_NAME="${CONTAINER_NAME:-manticore-manual-app}"
HOST_PORT="${HOST_PORT:-${PORT:-$DEFAULT_PORT}}"
READY_TIMEOUT="${READY_TIMEOUT:-120}"
MANUAL_SRC="$REPO_ROOT/manual"
DOC_APP_DIR="$REPO_ROOT/misc/doc-app-dir"
DOC_APP_REPO_URL="https://github.com/manticoresoftware/doc.git"
FORCE_NO_CACHE_BUILD=0
STOP_ONLY=0
CHECK_ONLY=0
MANUAL_POLL_INTERVAL="${MANUAL_POLL_INTERVAL:-5}"
USE_GITHUB_AUTH=0
GITHUB_AUTH_HEADER=""

error() {
  printf '[error] %s\n' "$*" >&2
  exit 1
}

if [[ -n "${GITHUB_USER:-}" || -n "${GITHUB_TOKEN:-}" ]]; then
  if [[ -z "${GITHUB_USER:-}" || -z "${GITHUB_TOKEN:-}" ]]; then
    error "GITHUB_USER and GITHUB_TOKEN must both be set when using authenticated Git access."
  fi
  if ! command -v base64 >/dev/null 2>&1; then
    error "Command 'base64' is required when using GITHUB_USER/TOKEN but was not found."
  fi
  USE_GITHUB_AUTH=1
  GITHUB_AUTH_HEADER="Authorization: Basic $(printf '%s:%s' "$GITHUB_USER" "$GITHUB_TOKEN" | base64 | tr -d '\n')"
fi

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
      --stop                Stop the running container and exit
      --check               Run doc validation (checkDocs) inside the container and exit

EOF
      exit 0
      ;;
    --stop)
      STOP_ONLY=1
      shift
      ;;
    --check)
      CHECK_ONLY=1
      shift
      ;;
    *)
      error "Unknown option: $1"
      ;;
  esac
done

if [[ "$STOP_ONLY" -eq 1 && "$CHECK_ONLY" -eq 1 ]]; then
  error "Options '--stop' and '--check' cannot be used together."
fi

run_docker() {
  if ! command -v docker >/dev/null 2>&1; then
    error "Command 'docker' is required but not found. Please install Docker and try again."
  fi
  if docker "$@"; then
    return 0
  fi

  if command -v sudo >/dev/null 2>&1; then
    echo "[warn] Docker command failed for current user; retrying with sudo..." >&2
    sudo docker "$@"
  else
    error "Docker command failed and 'sudo' is not available."
  fi
}

git_with_auth() {
  if [[ "$USE_GITHUB_AUTH" -eq 1 ]]; then
    if GIT_TERMINAL_PROMPT=0 git -c http.extraheader="$GITHUB_AUTH_HEADER" "$@"; then
      return 0
    fi
    printf "[error] GitHub authentication failed. Check GITHUB_USER/GITHUB_TOKEN in your environment or .env file.\n" >&2
    return 1
  else
    git "$@"
  fi
}

if [[ "$STOP_ONLY" -eq 1 ]]; then
  if run_docker ps -a --format '{{.Names}}' | grep -Fxq "$CONTAINER_NAME"; then
    printf "Stopping container '%s'..." "$CONTAINER_NAME"
    run_docker stop "$CONTAINER_NAME" >/dev/null
    printf "Done\n"
  else
    printf "Container '%s' is not running.\n" "$CONTAINER_NAME"
  fi
  exit 0
fi

if [[ "$CHECK_ONLY" -eq 1 ]]; then
  if run_docker ps --format '{{.Names}}' | grep -Fxq "$CONTAINER_NAME"; then
    printf "Running doc validation inside '%s'...\n" "$CONTAINER_NAME"
    run_docker exec "$CONTAINER_NAME" ./setup.sh -d . checkDocs
    echo "Doc validation completed."
  else
    error "Container '$CONTAINER_NAME' is not running; start it before using --check."
  fi
  exit 0
fi

if [[ ! -d "$MANUAL_SRC" ]]; then
  error "Manual directory '$MANUAL_SRC' does not exist"
fi
MANUAL_SRC="$(cd "$MANUAL_SRC" && pwd)"

CONTAINER_RUNNING=0
if run_docker ps --format '{{.Names}}' | grep -Fxq "$CONTAINER_NAME"; then
  CONTAINER_RUNNING=1
else
  if run_docker ps -a --format '{{.Names}}' | grep -Fxq "$CONTAINER_NAME"; then
    error "Container '$CONTAINER_NAME' already exists but is stopped; remove it or run with --stop first."
  fi
fi

if [[ "$CONTAINER_RUNNING" -eq 0 ]]; then
  if [[ ! -d "$DOC_APP_DIR/.git" ]]; then
    if ! command -v "git" >/dev/null 2>&1; then
      error "Command 'git' is required but not found. Please install Git and try again."
    fi
    rm -rf "$DOC_APP_DIR"
    git_with_auth clone --depth=1 "$DOC_APP_REPO_URL" "$DOC_APP_DIR"
    FORCE_NO_CACHE_BUILD=1
  else
    if ! command -v "git" >/dev/null 2>&1; then
      error "Command 'git' is required but not found. Please install Git and try again."
    fi
    CURRENT_HEAD="$(git -C "$DOC_APP_DIR" rev-parse HEAD)"
    git_with_auth -C "$DOC_APP_DIR" pull --ff-only
    UPDATED_HEAD="$(git -C "$DOC_APP_DIR" rev-parse HEAD)"
    if [[ "$CURRENT_HEAD" != "$UPDATED_HEAD" ]]; then
      FORCE_NO_CACHE_BUILD=1
    fi
  fi

  if [[ ! -f "$DOC_APP_DIR/Dockerfile" ]]; then
    error "No Dockerfile found in '$DOC_APP_DIR'; provide the doc app repo root"
  fi
fi

manual_snapshot() {
  git diff | cksum | awk '{print $1}'
}

monitor_manual_changes() {
  local last_snapshot
  last_snapshot="$(manual_snapshot)"
  printf "Watching '%s' for changes every %s seconds. Press Ctrl+C to stop watching.\n" "$MANUAL_SRC" "$MANUAL_POLL_INTERVAL"
  while true; do
    sleep "$MANUAL_POLL_INTERVAL"
    if ! run_docker ps --format '{{.Names}}' | grep -Fxq "$CONTAINER_NAME"; then
      echo "Container '$CONTAINER_NAME' is no longer running; stopping watcher."
      break
    fi
    local current_snapshot
    current_snapshot="$(manual_snapshot)"
    if [[ "$current_snapshot" != "$last_snapshot" ]]; then
      printf "Detected changes in '%s'; re-running setup inside container...\n" "$MANUAL_SRC"
      if run_docker exec "$CONTAINER_NAME" ./setup.sh >/dev/null; then
        run_docker exec "$CONTAINER_NAME" ./setup.sh -d . checkDocs
        echo "Setup and doc validation completed."
      else
        echo "[warn] Setup command failed; see output above." >&2
      fi
      last_snapshot="$current_snapshot"
    fi
  done
}

build_manual_image() {
  local extra_args=()
  if [[ "$FORCE_NO_CACHE_BUILD" -eq 1 ]]; then
    extra_args+=(--no-cache)
  fi

  if docker buildx version >/dev/null 2>&1; then
    # Ensure the image is loaded into the local Docker image store.
    run_docker buildx build --load "${extra_args[@]}" -t "$IMAGE_NAME" --build-arg DEPLOY_TARGET=local "$DOC_APP_DIR"
  else
    run_docker build "${extra_args[@]}" -t "$IMAGE_NAME" --build-arg DEPLOY_TARGET=local "$DOC_APP_DIR"
  fi
}

ready=0
if [[ "$CONTAINER_RUNNING" -eq 0 ]]; then
  build_manual_image

  printf "Starting container '$CONTAINER_NAME' on port $HOST_PORT..."
  run_docker run --rm -d \
    --name "$CONTAINER_NAME" \
    -p "$HOST_PORT":80 \
    -v "$MANUAL_SRC":/var/manual \
    "$IMAGE_NAME" >/dev/null

  for ((i=1; i<=READY_TIMEOUT; i++)); do
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
else
  ready=1
  printf "Container '%s' is already running.\nUse --stop to stop it.\n" "$CONTAINER_NAME"
fi

cat <<EOF

EOF

if [[ "$ready" -eq 1 ]]; then
  cat <<EOF
Manticore Manual app is running on http://localhost:${HOST_PORT}

To validate doc examples/links:
  ./manual.sh --check

To stop the container when you're done:
  ./manual.sh --stop

EOF
else
  cat <<EOF
Container '${CONTAINER_NAME}' started, but app did not become ready within ${READY_TIMEOUT}s.

Check logs:
  docker logs ${CONTAINER_NAME}

If this is expected on your machine, retry with a longer wait:
  READY_TIMEOUT=300 ./manual.sh

EOF
fi

monitor_manual_changes
