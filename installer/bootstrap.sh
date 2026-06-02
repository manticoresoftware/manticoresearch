#!/bin/bash

set -euo pipefail

print_usage() {
    cat <<'USAGE'
Manticore Search Installer

Usage:
  wget -qO- "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | bash -s -- [options]
  curl -sSL "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | bash -s -- [options]

Common commands/options:
  help, --help, -h, -?        Show this help and exit.
  silent, -s, yes, -y         Non-interactive mode; assume defaults.
  upgrade                     Upgrade an installed Manticore package.
  version <version>, -v <v>   Install or switch to a specific version.
  list-versions               Print available versions.
  list-versions-file <path>   Write available versions to path.
  no-start                    Do not start the service after install/upgrade.
  backup-data                 Include data directory in upgrade backup.
  no-backup-data              Skip data directory backup (default).
  backup-dir <path>           Override backup directory for upgrades.
  uninstall, -u               Remove packages, keep config/data/repo state.
  purge                       Remove packages and repository bootstrap package.
  purge-all                   Purge packages, repo state, config, and data.

Examples:
  sh bootstrap-standalone.sh list-versions
  sh bootstrap-standalone.sh version 25.0.0 no-start
  sh bootstrap-standalone.sh upgrade backup-data
USAGE
}
drain_stdin_if_piped() {
    if [[ ! -t 0 ]]; then
        while IFS= read -r _manticore_unused; do
            :
        done
    fi
}

exit_after_pipe_drain() {
    local status=$1
    drain_stdin_if_piped
    exit "$status"
}


ORIGINAL_ARGS=("$@")
while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help|-\?|help)
            print_usage
            exit_after_pipe_drain 0
            ;;
        -s|-y|silent|yes|no-start|backup-data|no-backup-data|-u|uninstall|purge|purge-all|upgrade|list-versions)
            shift
            ;;
        -v|version|backup-dir|list-versions-file)
            if [[ -z "${2:-}" || "${2:0:1}" == "-" ]]; then
                echo "[ERROR] $1 requires a value." >&2
                echo "Run with --help to see supported options." >&2
                exit_after_pipe_drain 2
            fi
            shift 2
            ;;
        list-versions-file=*)
            if [[ -z "${1#*=}" ]]; then
                echo "[ERROR] list-versions-file requires a value." >&2
                echo "Run with --help to see supported options." >&2
                exit_after_pipe_drain 2
            fi
            shift
            ;;
        *)
            echo "[ERROR] Unknown option: $1" >&2
            echo "Run with --help to see supported options." >&2
            exit_after_pipe_drain 2
            ;;
    esac
done
set -- "${ORIGINAL_ARGS[@]}"
unset ORIGINAL_ARGS

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
