#!/bin/bash

set -euo pipefail

print_usage() {
    cat <<'USAGE'
Manticore Search Installer

Usage:
  curl -sSL "$MANTICORE_INSTALLER_REPO_URL/bootstrap.sh" | bash -s -- [options]
  wget -qO- "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | bash -s -- [options]

Common options:
  -h, --help, -?              Show this help and exit.
  -s, --silent, -y, --yes     Non-interactive mode; assume defaults.
  --upgrade                   Upgrade an installed Manticore package.
  -v, --version <version>     Install or switch to a specific version.
  --list-versions [path]      Print available versions, or write them to path.
  --list-versions-file <path> Write available versions to path.
  --no-start                  Do not start the service after install/upgrade.
  --backup-data               Include data directory in upgrade backup.
  --no-backup-data            Skip data directory backup (default).
  --backup-dir <path>         Override backup directory for upgrades.
  -u, --uninstall             Remove packages, keep config/data/repo state.
  --purge                     Remove packages and repository bootstrap package.
  --purge-all                 Purge packages, repo state, config, and data.

Examples:
  bash bootstrap.sh --list-versions
  bash bootstrap.sh --version 25.0.0 --no-start
  bash bootstrap.sh --upgrade --backup-data
USAGE
}

for arg in "$@"; do
    case "$arg" in
        -h|--help|-\?)
            print_usage
            exit 0
            ;;
    esac
done

DEFAULT_REPO_URL="https://repo.manticoresearch.com/repository/install"
REPO_URL="${MANTICORE_INSTALLER_REPO_URL:-$DEFAULT_REPO_URL}"
TEMP_DIR=$(mktemp -d /tmp/manticore-installer.XXXXXX)
DEFAULT_INSTALL_LOG="/var/log/manticore_install.log"
if [[ -n "${MANTICORE_INSTALL_LOG:-}" ]]; then
    INSTALL_LOG="$MANTICORE_INSTALL_LOG"
elif [[ -w /var/log || ( -e "$DEFAULT_INSTALL_LOG" && -w "$DEFAULT_INSTALL_LOG" ) ]]; then
    INSTALL_LOG="$DEFAULT_INSTALL_LOG"
else
    INSTALL_LOG="${TMPDIR:-/tmp}/manticore_installer.log"
fi

cleanup() {
    rm -rf "$TEMP_DIR"
}

trap cleanup EXIT

COLOR_STRIP_REGEX='s/\x1b\[[0-9;]*m//g'
: > "$INSTALL_LOG"

if [[ -t 1 ]]; then
    exec > >(tee /dev/tty | sed "$COLOR_STRIP_REGEX" >> "$INSTALL_LOG") 2>&1
else
    exec > >(tee -a "$INSTALL_LOG") 2>&1
fi

print_log() {
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] $1"
}

download_file() {
    local url=$1
    local dest=$2

    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$url" -o "$dest"
        return $?
    elif command -v wget >/dev/null 2>&1; then
        wget -qO "$dest" "$url"
        return $?
    else
        return 127
    fi
}

if ! command -v curl >/dev/null 2>&1 && ! command -v wget >/dev/null 2>&1; then
    print_log "ERROR: Neither 'curl' nor 'wget' was found on this system."
    print_log "Please install one of them to proceed: 'apt-get install curl' or 'yum install wget'"
    exit 1
fi

print_log "Initializing Manticore Search Installer..."
print_log "Logs are being written to $INSTALL_LOG"
print_log "Using installer source: $REPO_URL"

MODULES=("constants.sh" "detect.sh" "ui.sh" "install.sh" "upgrade.sh" "main.sh" "uninstall.sh")

for mod in "${MODULES[@]}"; do
    print_log "Downloading $mod..."
    if ! download_file "${REPO_URL}/${mod}" "${TEMP_DIR}/${mod}"; then
        print_log "Error: Failed to download ${mod} using available tools."
        exit 1
    fi
done

cd "$TEMP_DIR"
bash ./main.sh "$@"
print_log "Installer log saved to $INSTALL_LOG"
