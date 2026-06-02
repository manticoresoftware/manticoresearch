#!/bin/bash

set -euo pipefail

SCRIPT_DIR=$(dirname -- "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(cd "$SCRIPT_DIR" && pwd)
source "$SCRIPT_DIR/constants.sh"
source "$SCRIPT_DIR/detect.sh"
source "$SCRIPT_DIR/ui.sh"

if [[ "${MANTICORE_STANDALONE:-0}" == "1" ]]; then
    ACTION_MODE="uninstall"
else
    ACTION_MODE="${1:-uninstall}"
fi

ensure_service_stopped() {
    if service_is_active; then
        print_step "Stopping Manticore Service"
        stop_service
    else
        print_info "Manticore service is already stopped."
    fi
}

remove_package() {
    local package_specs=()

    if ! package_installed; then
        print_info "Manticore package is already absent."
        return 0
    fi

    print_step "Removing Manticore Package"

    if [[ "$OS_FAMILY" == "debian" ]]; then
        read -r -a package_specs <<< "$DEB_FALLBACK_VERSIONED_PACKAGE_NAMES"
        sudo_exec apt-get remove -y "${package_specs[@]}"
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        read -r -a package_specs <<< "$RPM_FALLBACK_VERSIONED_PACKAGE_NAMES"
        if command -v dnf >/dev/null 2>&1; then
            sudo_exec dnf remove -y "${package_specs[@]}"
        else
            sudo_exec yum remove -y "${package_specs[@]}"
        fi
    elif [[ "$OS_FAMILY" == "brew" ]]; then
        brew uninstall "$BREW_SERVICE_NAME"
    fi
}

remove_repo_package() {
    if [[ "$OS_FAMILY" == "brew" ]]; then
        print_info "Homebrew installation has no Manticore repository package to remove."
        return 0
    fi

    if ! repo_package_installed; then
        print_info "Repository bootstrap package is already absent."
        return 0
    fi

    print_step "Removing Repository Bootstrap Package"

    if [[ "$OS_FAMILY" == "debian" ]]; then
        sudo_exec apt-get remove -y "$DEB_REPO_PACKAGE_NAME"
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        if command -v dnf >/dev/null 2>&1; then
            sudo_exec dnf remove -y "$RPM_REPO_PACKAGE_NAME"
        else
            sudo_exec yum remove -y "$RPM_REPO_PACKAGE_NAME"
        fi
    fi
}

confirm_purge_all() {
    if [[ "$ACTION_MODE" != "purge-all" || "${SILENT:-false}" == "true" ]]; then
        return 0
    fi

    if ! prompt_tty_available; then
        print_error "purge-all removes $CONF_DIR and $DATA_DIR. Rerun with silent to confirm this non-interactively."
        return 1
    fi

    local response
    {
        echo "This will remove Manticore configuration and data directories:"
        echo "  $CONF_DIR"
        echo "  $DATA_DIR"
        printf "%s" "Type DELETE to continue: "
    } > /dev/tty

    if ! IFS= read -r response < /dev/tty; then
        print_error "Could not read confirmation from terminal."
        return 1
    fi

    if [[ "$response" != "DELETE" ]]; then
        print_info "Purge cancelled."
        exit 0
    fi
}

purge_state_dirs() {
    local path

    print_step "Purging Manticore State"
    for path in "$DATA_DIR" "$CONF_DIR"; do
        if [[ -e "$path" ]]; then
            sudo_exec rm -rf "$path"
            print_info "Removed $path"
        else
            print_info "$path is already absent."
        fi
    done
}

uninstall_flow() {
    ACTION_MODE=${1:-$ACTION_MODE}

    confirm_purge_all
    ensure_service_stopped
    remove_package

    if [[ "$ACTION_MODE" == "purge" || "$ACTION_MODE" == "purge-all" ]]; then
        remove_repo_package
    fi

    if [[ "$ACTION_MODE" == "purge-all" ]]; then
        purge_state_dirs
    fi

    print_success "Uninstall completed."
}

uninstall_main() {
    detect_os

    if [[ "$OS_FAMILY" == "unknown" ]]; then
        print_unsupported_os
        exit 1
    fi

    uninstall_flow "$ACTION_MODE"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" && "${MANTICORE_STANDALONE:-0}" != "1" ]]; then
    uninstall_main "$@"
fi
