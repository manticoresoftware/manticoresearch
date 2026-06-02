#!/bin/bash

set -euo pipefail

SCRIPT_DIR=$(dirname -- "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(cd "$SCRIPT_DIR" && pwd)
OUTPUT_PATH=${1:-"$SCRIPT_DIR/bootstrap-standalone.sh"}
TMP_PATH=$(mktemp "${OUTPUT_PATH}.XXXXXX")
BASH_TMP_PATH=$(mktemp "${OUTPUT_PATH}.bash.XXXXXX")

cleanup() {
    rm -f "$TMP_PATH"
    rm -f "$BASH_TMP_PATH"
}
trap cleanup EXIT

append_module() {
    local module_path=$1

    awk '
        skip_self_run && /^[[:space:]]*fi[[:space:]]*$/ { skip_self_run=0; next }
        skip_self_run { next }
        NR == 1 && /^#!/ { next }
        /^set -euo pipefail$/ { next }
        /SCRIPT_DIR/ { next }
        /BASH_SOURCE/ { skip_self_run=1; next }
        { print }
    ' "$module_path"
}

{
    cat <<'HEADER'
#!/bin/bash

set -euo pipefail

MANTICORE_STANDALONE=1

standalone_print_usage() {
    cat <<'USAGE'
Manticore Search Installer

Usage:
  wget -qO- "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | sh -s -- [options]
  curl -sSL "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | sh -s -- [options]
  wget -qO- "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | bash -s -- [options]
  curl -sSL "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | bash -s -- [options]

Common options:
  -h, --help, -?              Show this help and exit.
  -s, --silent, -y, --yes     Non-interactive mode; assume defaults.
  --upgrade                   Upgrade an installed Manticore package.
  -v, --version <version>     Install or switch to a specific version.
  --list-versions             Print available versions.
  --list-versions-file <path> Write available versions to path.
  --no-start                  Do not start the service after install/upgrade.
  --backup-data               Include data directory in upgrade backup.
  --no-backup-data            Skip data directory backup (default).
  --backup-dir <path>         Override backup directory for upgrades.
  -u, --uninstall             Remove packages, keep config/data/repo state.
  --purge                     Remove packages and repository bootstrap package.
  --purge-all                 Purge packages, repo state, config, and data.

Examples:
  bash bootstrap-standalone.sh --list-versions
  bash bootstrap-standalone.sh --version 25.0.0 --no-start
  bash bootstrap-standalone.sh --upgrade --backup-data
USAGE
}

standalone_drain_stdin_if_piped() {
    if [[ ! -t 0 ]]; then
        while IFS= read -r _manticore_unused; do
            :
        done
    fi
}

standalone_exit_after_pipe_drain() {
    local status=$1
    standalone_drain_stdin_if_piped
    exit "$status"
}

standalone_usage_error() {
    echo "[ERROR] $1" >&2
    echo "Run with --help to see supported options." >&2
    standalone_exit_after_pipe_drain 2
}

standalone_validate_args() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -h|--help|-\?)
                standalone_print_usage
                standalone_exit_after_pipe_drain 0
                ;;
            -s|--silent|-y|--yes|--no-start|--backup-data|--no-backup-data|-u|--uninstall|--purge|--purge-all|--upgrade|--list-versions)
                shift
                ;;
            -v|--version|--backup-dir|--list-versions-file)
                if [[ -z "${2:-}" || "${2:0:1}" == "-" ]]; then
                    standalone_usage_error "$1 requires a value."
                fi
                shift 2
                ;;
            --list-versions-file=*)
                if [[ -z "${1#*=}" ]]; then
                    standalone_usage_error "--list-versions-file requires a value."
                fi
                shift
                ;;
            *)
                standalone_usage_error "Unknown option: $1"
                ;;
        esac
    done
}

standalone_validate_args "$@"
unset -f standalone_validate_args standalone_usage_error standalone_print_usage standalone_exit_after_pipe_drain standalone_drain_stdin_if_piped
HEADER

    echo ""
    echo "# ---- constants.sh ----"
    append_module "$SCRIPT_DIR/constants.sh"

    cat <<'LOGGING'

# ---- standalone logging ----
DEFAULT_INSTALL_LOG="/var/log/manticore_install.log"
if [[ -n "${MANTICORE_INSTALL_LOG:-}" ]]; then
    INSTALL_LOG="$MANTICORE_INSTALL_LOG"
elif [[ -w /var/log || ( -e "$DEFAULT_INSTALL_LOG" && -w "$DEFAULT_INSTALL_LOG" ) ]]; then
    INSTALL_LOG="$DEFAULT_INSTALL_LOG"
else
    INSTALL_LOG="${TMPDIR:-/tmp}/manticore_installer.log"
fi

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
LOGGING

    for module in ui.sh upgrade.sh install.sh uninstall.sh detect.sh main.sh; do
        echo ""
        echo "# ---- ${module} ----"
        append_module "$SCRIPT_DIR/$module"
    done
} > "$BASH_TMP_PATH"

bash -n "$BASH_TMP_PATH"

{
    cat <<'SH_WRAPPER'
#!/bin/sh
set -eu

if ! command -v bash >/dev/null 2>&1; then
    echo "[ERROR] bash is required to run this installer." >&2
    exit 1
fi

payload_path=$(mktemp "${TMPDIR:-/tmp}/manticore-standalone.XXXXXX.sh")

cleanup_payload() {
    rm -f "$payload_path"
}
trap cleanup_payload EXIT HUP INT TERM

cat > "$payload_path" <<'MANTICORE_STANDALONE_BASH_PAYLOAD'
SH_WRAPPER

    cat "$BASH_TMP_PATH"

    cat <<'SH_WRAPPER'
MANTICORE_STANDALONE_BASH_PAYLOAD

bash "$payload_path" "$@"
SH_WRAPPER
} > "$TMP_PATH"

chmod +x "$TMP_PATH"
sh -n "$TMP_PATH"
mv "$TMP_PATH" "$OUTPUT_PATH"
trap - EXIT
rm -f "$TMP_PATH" "$BASH_TMP_PATH"
echo "Built $OUTPUT_PATH"
