#!/bin/bash

set -euo pipefail

SCRIPT_DIR=$(dirname -- "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(cd "$SCRIPT_DIR" && pwd)
source "$SCRIPT_DIR/constants.sh"
source "$SCRIPT_DIR/detect.sh"
source "$SCRIPT_DIR/ui.sh"

SILENT=false
if [[ "${MANTICORE_STANDALONE:-0}" == "1" ]]; then
    REQUESTED_VERSION=""
else
    REQUESTED_VERSION="${1:-}"
fi
MANTICORE_START_SERVICE="${MANTICORE_START_SERVICE:-true}"

download_with_available_tool() {
    local url=$1
    local dest=$2

    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$url" -o "$dest"
    else
        wget -qO "$dest" "$url"
    fi
}

install_repo_package() {
    ensure_repo_channel
}

install_manticore_package() {
    local target_version=${1:-}
    local package_version=""
    local package_specs=()

    if desired_version_installed "$target_version"; then
        if [[ -n "$target_version" ]]; then
            print_info "Requested version $target_version is already installed."
        else
            print_info "Manticore is already installed."
        fi
        return 0
    fi

    if [[ -n "$target_version" && "$OS_FAMILY" != "brew" ]]; then
        package_version=$(resolve_requested_version "$target_version") || return 1
        ensure_versioned_package_set_available "$package_version" || return 1
    fi

    print_step "Installing Manticore"

    if [[ "$OS_FAMILY" == "debian" ]]; then
        if [[ -n "$package_version" ]]; then
            mapfile -t package_specs < <(versioned_debian_package_specs "$package_version")
            apt_get_install_with_repo_repair -y --allow-downgrades "${package_specs[@]}"
            mark_debian_split_packages_auto_for_thin_version "$package_version"
        else
            apt_get_install_with_repo_repair -y "$PACKAGE_NAME"
        fi
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        if command -v dnf >/dev/null 2>&1; then
            if [[ -n "$package_version" ]]; then
                prepare_rpm_packages_for_version "$package_version"
                mapfile -t package_specs < <(versioned_rpm_package_specs "$package_version")
                rpm_sudo_run dnf install -y "${package_specs[@]}"
            else
                rpm_sudo_run dnf install -y "$PACKAGE_NAME"
            fi
        else
            if [[ -n "$package_version" ]]; then
                prepare_rpm_packages_for_version "$package_version"
                mapfile -t package_specs < <(versioned_rpm_package_specs "$package_version")
                rpm_sudo_run yum install -y "${package_specs[@]}"
            else
                rpm_sudo_run yum install -y "$PACKAGE_NAME"
            fi
        fi
    elif [[ "$OS_FAMILY" == "brew" ]]; then
        if [[ -n "$target_version" ]]; then
            print_error "version is not supported for Homebrew installs."
            return 1
        fi
        brew install "$BREW_PACKAGE_NAME"
    fi

    print_success "Manticore package is installed."
}


ensure_service_started() {
    if [[ "$MANTICORE_START_SERVICE" != "true" ]]; then
        print_info "Skipping service start because no-start was requested."
        return 0
    fi

    if service_is_active; then
        print_info "Service is already running."
        return 0
    fi

    ensure_default_port_available
    print_step "Starting Manticore Service"
    start_service
    print_success "Service started."
}

verify_installation() {
    print_step "Verification"

    if ! command -v searchd >/dev/null 2>&1; then
        print_error "Binary 'searchd' not found after installation."
        return 1
    fi

    if service_is_active; then
        print_success "Manticore is running."
    elif service_status_observable; then
        print_warn "Manticore package is installed but service is not active."
    else
        print_warn "Manticore package is installed, but service state could not be checked because this system lacks systemd, pgrep, and ps."
    fi
}

install_flow() {
    local requested_version=${1:-$REQUESTED_VERSION}
    validate_requested_version_argument "$requested_version" version

    warn_about_manual_installation
    ensure_repo_channel
    refresh_package_metadata
    install_manticore_package "$requested_version"
    ensure_service_started
    verify_installation
}

install_main() {
    validate_requested_version_argument "$REQUESTED_VERSION" version
    detect_os
    detect_arch

    if [[ "$OS_FAMILY" == "unknown" ]]; then
        print_unsupported_os
        exit 1
    fi

    if ! is_supported_arch; then
        print_error "Unsupported architecture: $(uname -m)"
        exit 1
    fi

    install_flow "$REQUESTED_VERSION"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" && "${MANTICORE_STANDALONE:-0}" != "1" ]]; then
    install_main "$@"
fi
