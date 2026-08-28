#!/bin/bash

set -euo pipefail

SCRIPT_DIR=$(dirname -- "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(cd "$SCRIPT_DIR" && pwd)
source "$SCRIPT_DIR/constants.sh"
source "$SCRIPT_DIR/ui.sh"
source "$SCRIPT_DIR/detect.sh"

SILENT=false
SPECIFIC_VERSION=""
ACTION=""
UPGRADE_REQUESTED=false
MANTICORE_START_SERVICE=true
MANTICORE_BACKUP_DATA=no
MANTICORE_BACKUP_DIR=""
MANTICORE_REPO_CHANNEL=""
LIST_VERSIONS_OUTPUT_FILE=""

usage_init_styles() {
    if use_color; then
        USAGE_TITLE_STYLE="$BOLD"
        USAGE_SECTION_STYLE="${BOLD}${CYAN}"
        USAGE_COMMAND_STYLE="$BOLD"
        USAGE_EXAMPLE_STYLE="$USAGE_COMMAND_STYLE"
        USAGE_PARAM_STYLE="$CYAN"
        USAGE_RESET="$NC"
    else
        USAGE_TITLE_STYLE=""
        USAGE_SECTION_STYLE=""
        USAGE_COMMAND_STYLE=""
        USAGE_EXAMPLE_STYLE=""
        USAGE_PARAM_STYLE=""
        USAGE_RESET=""
    fi
}

usage_title() {
    printf "%b%s%b\n\n" "$USAGE_TITLE_STYLE" "$1" "$USAGE_RESET"
}

usage_section() {
    printf "%b%s%b\n" "$USAGE_SECTION_STYLE" "$1" "$USAGE_RESET"
}

usage_print_styled_text() {
    local text=$1
    local base_style=$2
    local remaining prefix token

    if [[ -z "$USAGE_RESET" ]]; then
        printf "%s" "$text"
        return 0
    fi

    remaining=$text
    printf "%b" "$base_style"
    while [[ "$remaining" =~ ^([^<[]*)(<[^>]+>|\[[^]]+\])(.*)$ ]]; do
        prefix=${BASH_REMATCH[1]}
        token=${BASH_REMATCH[2]}
        remaining=${BASH_REMATCH[3]}
        printf "%s%b%s%b" "$prefix" "$USAGE_PARAM_STYLE" "$token" "$base_style"
    done
    printf "%s%b" "$remaining" "$USAGE_RESET"
}

usage_line() {
    local left=$1
    local right=$2
    local padding=$((27 - ${#left}))
    [[ $padding -lt 0 ]] && padding=0

    printf "  "
    usage_print_styled_text "$left" "$USAGE_COMMAND_STYLE"
    printf "%*s %s\n" "$padding" "" "$right"
}

usage_example() {
    printf "  "
    usage_print_styled_text "$1" "$USAGE_EXAMPLE_STYLE"
    printf "\n"
}

print_common_usage_body() {
    usage_section "Common commands/options:"
    usage_line "help" "Show this help and exit."
    usage_line "silent, yes" "Non-interactive mode; assume defaults."
    usage_line "upgrade" "Upgrade an installed Manticore package."
    usage_line "version <version>" "Install or switch to a specific version."
    usage_line "list-versions" "Print available versions."
    usage_line "list-versions-file <path>" "Write available versions to path."
    usage_line "no-start" "Do not start the service after install/upgrade."
    usage_line "backup-data" "Include data directory in upgrade backup."
    usage_line "no-backup-data" "Skip data directory backup (default)."
    usage_line "backup-dir <path>" "Override backup directory for upgrades."
    usage_line "release, dev" "Select Manticore repository channel."
    usage_line "uninstall" "Remove packages, keep config/data/repo state."
    usage_line "purge" "Remove packages and repository bootstrap package."
    usage_line "purge-all" "Purge packages, repo state, config, and data."
}

print_standalone_usage() {
    usage_init_styles
    usage_title "Manticore Search Installer"
    usage_section "Usage:"
    usage_example "wget -O- https://manticoresearch.com | sh -s [options]"
    usage_example "curl https://manticoresearch.com | sh -s [options]"
    printf "\n"
    print_common_usage_body
    printf "\n"
    usage_section "Examples:"
    usage_example "curl https://manticoresearch.com | sh -s list-versions"
    usage_example "curl https://manticoresearch.com | sh -s dev list-versions"
    usage_example "curl https://manticoresearch.com | sh -s version 25.0.0 no-start"
    usage_example "curl https://manticoresearch.com | sh -s upgrade backup-data"
}

print_modular_usage() {
    usage_init_styles
    usage_title "Manticore Search Installer"
    usage_section "Usage:"
    usage_example 'wget -qO- "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | bash -s -- [options]'
    usage_example 'curl -sSL "$MANTICORE_INSTALLER_REPO_URL/bootstrap-standalone.sh" | bash -s -- [options]'
    printf "\n"
    print_common_usage_body
    printf "\n"
    usage_section "Examples:"
    usage_example "sh bootstrap-standalone.sh list-versions"
    usage_example "sh bootstrap-standalone.sh dev list-versions"
    usage_example "sh bootstrap-standalone.sh version 25.0.0 no-start"
    usage_example "sh bootstrap-standalone.sh upgrade backup-data"
}

print_usage() {
    if [[ "${MANTICORE_STANDALONE:-0}" == "1" ]]; then
        print_standalone_usage
    else
        print_modular_usage
    fi
}
usage_error() {
    print_error "$1"
    echo "Run with --help to see supported options." >&2
    exit 2
}



set_requested_repo_channel() {
    local normalized

    normalized="$1" || usage_error "Unknown repository channel: $1"
    if [[ -n "$MANTICORE_REPO_CHANNEL" && "$MANTICORE_REPO_CHANNEL" != "$normalized" ]]; then
        usage_error "Conflicting repository channels: $MANTICORE_REPO_CHANNEL and $normalized"
    fi
    MANTICORE_REPO_CHANNEL="$normalized"
}

parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help|-\?|help)
                print_usage
                exit 0
                ;;
            -s|-y|silent|yes)
                SILENT=true
                shift
                ;;
            no-start)
                MANTICORE_START_SERVICE=false
                shift
                ;;
            backup-data)
                MANTICORE_BACKUP_DATA=yes
                shift
                ;;
            no-backup-data)
                MANTICORE_BACKUP_DATA=no
                shift
                ;;
            release|dev)
                set_requested_repo_channel "$1"
                shift
                ;;
            backup-dir)
                if [[ -z "${2:-}" || "${2:0:1}" == "-" ]]; then
                    usage_error "backup-dir requires a value."
                fi
                MANTICORE_BACKUP_DIR="$2"
                shift 2
                ;;
            -v|version)
                if [[ -z "${2:-}" || "${2:0:1}" == "-" ]]; then
                    usage_error "version requires a value."
                fi
                SPECIFIC_VERSION="$2"
                shift 2
                ;;
            -u|uninstall)
                ACTION="uninstall"
                shift
                ;;
            purge)
                ACTION="purge"
                shift
                ;;
            purge-all)
                ACTION="purge-all"
                shift
                ;;
            upgrade)
                UPGRADE_REQUESTED=true
                shift
                ;;
            list-versions)
                ACTION="list-versions"
                shift
                ;;
            list-versions-file)
                if [[ -z "${2:-}" || "${2:0:1}" == "-" ]]; then
                    usage_error "$1 requires a value."
                fi
                ACTION="list-versions"
                LIST_VERSIONS_OUTPUT_FILE="$2"
                shift 2
                ;;
            list-versions-file=*)
                ACTION="list-versions"
                LIST_VERSIONS_OUTPUT_FILE="${1#*=}"
                if [[ -z "$LIST_VERSIONS_OUTPUT_FILE" ]]; then
                    usage_error "list-versions-file requires a value."
                fi
                shift
                ;;
            *)
                usage_error "Unknown option: $1"
                ;;
        esac
    done
}


get_target_version() {
    if [[ -n "$SPECIFIC_VERSION" ]]; then
        echo "$SPECIFIC_VERSION"
    else
        get_latest_available_version
    fi
}

refine_version_list() {
    awk 'NF {print $1}' | sort -Vu
}

write_versions_output_file() {
    local output_file=$1
    local versions=$2

    if [[ -z "$output_file" ]]; then
        printf '%s\n' "$versions"
        return 0
    fi

    if ! printf '%s\n' "$versions" > "$output_file"; then
        print_error "Failed to write available versions to $output_file."
        return 1
    fi

    print_info "Available Manticore versions written to $output_file."
}

list_versions_action() {
    local versions

    ensure_repo_channel
    refresh_package_metadata
    versions=$(list_available_versions | refine_version_list || true)
    if [[ -z "$versions" ]]; then
        print_error "No available Manticore versions were found for this package family."
        return 1
    fi

    write_versions_output_file "$LIST_VERSIONS_OUTPUT_FILE" "$versions"
}

determine_action() {
    local current_version=""
    local target_version=""

    if [[ -n "$ACTION" ]]; then
        return 0
    fi

    if ! package_installed; then
        ACTION="install"
        return 0
    fi

    if [[ "$UPGRADE_REQUESTED" == "true" && -z "$SPECIFIC_VERSION" && -z "$MANTICORE_REPO_CHANNEL" ]]; then
        ACTION="upgrade"
        return 0
    fi

    ensure_repo_channel
    refresh_package_metadata
    current_version=$(get_installed_version || true)
    target_version=$(get_target_version || true)

    if [[ -n "$SPECIFIC_VERSION" ]]; then
        if desired_version_installed "$SPECIFIC_VERSION"; then
            print_info "Requested version $SPECIFIC_VERSION is already installed."
            exit 0
        fi

        print_info "Switching from version $current_version to requested version $SPECIFIC_VERSION."
        ACTION="upgrade"
        return 0
    fi

    if [[ -z "$target_version" ]]; then
        print_warn "Could not determine the latest available version."
        print_info "Installed version is $current_version."
        exit 0
    fi

    if desired_version_installed "$target_version"; then
        if [[ -n "$MANTICORE_REPO_CHANNEL" ]]; then
            if [[ "$OS_FAMILY" == "brew" ]]; then
                print_info "Installed version $current_version is already the latest available in the Homebrew ${MANTICORE_REPO_CHANNEL} formula."
            else
                print_info "Installed version $current_version is already the latest available in the ${MANTICORE_REPO_CHANNEL} repository."
            fi
        else
            print_info "Installed version $current_version is already up to date."
        fi
        exit 0
    fi

    if [[ -n "$MANTICORE_REPO_CHANNEL" ]]; then
        if [[ "$OS_FAMILY" == "brew" ]]; then
            if [[ "$UPGRADE_REQUESTED" == "true" ]] || ask_confirm "Latest Homebrew ${MANTICORE_REPO_CHANNEL} formula version is $target_version. Upgrade from $current_version?"; then
                print_info "Switching from version $current_version to latest Homebrew ${MANTICORE_REPO_CHANNEL} formula version $target_version."
                ACTION="upgrade"
                return 0
            else
                print_info "Leaving version $current_version installed."
                exit 0
            fi
        fi

        if [[ "$UPGRADE_REQUESTED" == "true" ]] || ask_confirm "Latest ${MANTICORE_REPO_CHANNEL} repository version is $target_version. Switch from $current_version?"; then
            print_info "Switching from version $current_version to latest ${MANTICORE_REPO_CHANNEL} repository version $target_version."
            SPECIFIC_VERSION="$target_version"
            ACTION="upgrade"
            return 0
        else
            print_info "Leaving version $current_version installed."
            exit 0
        fi
    fi

    if ! version_gt "$target_version" "$current_version"; then
        print_info "Installed version $current_version is already up to date."
        exit 0
    fi

    if [[ "$UPGRADE_REQUESTED" == "true" ]]; then
        ACTION="upgrade"
        return 0
    fi

    if [[ "$SILENT" == "true" ]]; then
        print_info "Newer version $target_version is available. Rerun with upgrade to apply it."
        exit 0
    fi

    if ask_confirm "Version $target_version is available. Upgrade from $current_version?"; then
        ACTION="upgrade"
    else
        print_info "Leaving version $current_version installed."
        exit 0
    fi
}

execute_standalone_action() {
    case "$ACTION" in
        install) install_flow "$SPECIFIC_VERSION" ;;
        upgrade) upgrade_flow "$SPECIFIC_VERSION" ;;
        uninstall|purge|purge-all) uninstall_flow "$ACTION" ;;
        list-versions) list_versions_action ;;
        *)
            print_error "No action selected."
            exit 1
            ;;
    esac
}

execute_modular_action() {
    case "$ACTION" in
        install)
            bash "$SCRIPT_DIR/install.sh" "$SPECIFIC_VERSION"
            ;;
        upgrade)
            bash "$SCRIPT_DIR/upgrade.sh" "$SPECIFIC_VERSION"
            ;;
        uninstall)
            bash "$SCRIPT_DIR/uninstall.sh" uninstall
            ;;
        purge)
            bash "$SCRIPT_DIR/uninstall.sh" purge
            ;;
        purge-all)
            bash "$SCRIPT_DIR/uninstall.sh" purge-all
            ;;
        list-versions)
            list_versions_action
            ;;
        *)
            print_error "No action selected."
            exit 1
            ;;
    esac
}

execute_action() {
    if [[ "${MANTICORE_STANDALONE:-0}" == "1" ]]; then
        execute_standalone_action
    else
        execute_modular_action
    fi
}
main() {
    parse_args "$@"
    if [[ "${MANTICORE_STANDALONE:-0}" == "1" ]]; then
        print_log "Initializing self-contained Manticore Search Installer..."
        print_log "Logs are being written to $INSTALL_LOG"
    fi

    export SILENT MANTICORE_START_SERVICE MANTICORE_BACKUP_DATA MANTICORE_BACKUP_DIR MANTICORE_REPO_CHANNEL
    detect_os
    detect_arch

    if [[ "$OS_FAMILY" == "unknown" ]]; then
        print_unsupported_os
        exit 1
    fi

    if [[ "$ACTION" != "uninstall" && "$ACTION" != "purge" && "$ACTION" != "purge-all" && "$ACTION" != "list-versions" ]] && ! is_supported_arch; then
        print_error "Unsupported architecture: $(uname -m)"
        exit 1
    fi

    determine_action
    execute_action

    if [[ "${MANTICORE_STANDALONE:-0}" == "1" ]]; then
        print_log "Installer log saved to $INSTALL_LOG"
    fi
}

main "$@"
