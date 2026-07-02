#!/bin/bash

set -euo pipefail

BOOTSTRAP_CYAN='\033[0;36m'
BOOTSTRAP_BOLD='\033[1m'
BOOTSTRAP_NC='\033[0m'

bootstrap_use_color() {
    [[ -z "${NO_COLOR:-}" ]] || return 1

    if [[ "${MANTICORE_FORCE_COLOR:-}" == "1" || "${CLICOLOR_FORCE:-}" == "1" ]]; then
        return 0
    fi

    [[ -t 1 ]]
}

bootstrap_usage_init_styles() {
    if bootstrap_use_color; then
        BOOTSTRAP_USAGE_TITLE_STYLE="$BOOTSTRAP_BOLD"
        BOOTSTRAP_USAGE_SECTION_STYLE="${BOOTSTRAP_BOLD}${BOOTSTRAP_CYAN}"
        BOOTSTRAP_USAGE_COMMAND_STYLE="$BOOTSTRAP_BOLD"
        BOOTSTRAP_USAGE_EXAMPLE_STYLE="$BOOTSTRAP_USAGE_COMMAND_STYLE"
        BOOTSTRAP_USAGE_PARAM_STYLE="$BOOTSTRAP_CYAN"
        BOOTSTRAP_USAGE_RESET="$BOOTSTRAP_NC"
    else
        BOOTSTRAP_USAGE_TITLE_STYLE=""
        BOOTSTRAP_USAGE_SECTION_STYLE=""
        BOOTSTRAP_USAGE_COMMAND_STYLE=""
        BOOTSTRAP_USAGE_EXAMPLE_STYLE=""
        BOOTSTRAP_USAGE_PARAM_STYLE=""
        BOOTSTRAP_USAGE_RESET=""
    fi
}

bootstrap_usage_title() {
    printf "%b%s%b\n\n" "$BOOTSTRAP_USAGE_TITLE_STYLE" "$1" "$BOOTSTRAP_USAGE_RESET"
}

bootstrap_usage_section() {
    printf "%b%s%b\n" "$BOOTSTRAP_USAGE_SECTION_STYLE" "$1" "$BOOTSTRAP_USAGE_RESET"
}

bootstrap_usage_print_styled_text() {
    local text=$1
    local base_style=$2
    local remaining prefix token

    if [[ -z "$BOOTSTRAP_USAGE_RESET" ]]; then
        printf "%s" "$text"
        return 0
    fi

    remaining=$text
    printf "%b" "$base_style"
    while [[ "$remaining" =~ ^([^<[]*)(<[^>]+>|\[[^]]+\])(.*)$ ]]; do
        prefix=${BASH_REMATCH[1]}
        token=${BASH_REMATCH[2]}
        remaining=${BASH_REMATCH[3]}
        printf "%s%b%s%b" "$prefix" "$BOOTSTRAP_USAGE_PARAM_STYLE" "$token" "$base_style"
    done
    printf "%s%b" "$remaining" "$BOOTSTRAP_USAGE_RESET"
}

bootstrap_usage_line() {
    local left=$1
    local right=$2
    local padding=$((27 - ${#left}))
    [[ $padding -lt 0 ]] && padding=0

    printf "  "
    bootstrap_usage_print_styled_text "$left" "$BOOTSTRAP_USAGE_COMMAND_STYLE"
    printf "%*s %s\n" "$padding" "" "$right"
}

bootstrap_usage_example() {
    printf "  "
    bootstrap_usage_print_styled_text "$1" "$BOOTSTRAP_USAGE_EXAMPLE_STYLE"
    printf "\n"
}

print_usage() {
    bootstrap_usage_init_styles
    bootstrap_usage_title "Manticore Search Installer"
    bootstrap_usage_section "Usage:"
    bootstrap_usage_example 'wget -qO- "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | bash -s -- [options]'
    bootstrap_usage_example 'curl -sSL "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | bash -s -- [options]'
    printf "\n"
    bootstrap_usage_section "Common commands/options:"
    bootstrap_usage_line "help" "Show this help and exit."
    bootstrap_usage_line "silent, yes" "Non-interactive mode; assume defaults."
    bootstrap_usage_line "upgrade" "Upgrade an installed Manticore package."
    bootstrap_usage_line "version <version>" "Install or switch to a specific version."
    bootstrap_usage_line "list-versions" "Print available versions."
    bootstrap_usage_line "list-versions-file <path>" "Write available versions to path."
    bootstrap_usage_line "no-start" "Do not start the service after install/upgrade."
    bootstrap_usage_line "backup-data" "Include data directory in upgrade backup."
    bootstrap_usage_line "no-backup-data" "Skip data directory backup (default)."
    bootstrap_usage_line "backup-dir <path>" "Override backup directory for upgrades."
    bootstrap_usage_line "release, dev" "Select Manticore repository channel."
    bootstrap_usage_line "uninstall" "Remove packages, keep config/data/repo state."
    bootstrap_usage_line "purge" "Remove packages and repository bootstrap package."
    bootstrap_usage_line "purge-all" "Purge packages, repo state, config, and data."
    printf "\n"
    bootstrap_usage_section "Examples:"
    bootstrap_usage_example "sh bootstrap-standalone.sh list-versions"
    bootstrap_usage_example "sh bootstrap-standalone.sh dev list-versions"
    bootstrap_usage_example "sh bootstrap-standalone.sh version 25.0.0 no-start"
    bootstrap_usage_example "sh bootstrap-standalone.sh upgrade backup-data"
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
        -s|-y|silent|yes|no-start|backup-data|no-backup-data|-u|uninstall|purge|purge-all|upgrade|list-versions|release|dev)
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
    export MANTICORE_COLOR_TTY=true
    exec > >(tee /dev/tty | sed "$COLOR_STRIP_REGEX" >> "$INSTALL_LOG") 2>&1
else
    export MANTICORE_COLOR_TTY=false
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
