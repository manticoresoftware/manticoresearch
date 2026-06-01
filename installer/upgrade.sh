#!/bin/bash

set -euo pipefail

if [[ "${MANTICORE_UPGRADE_MODULE:-0}" != "1" && "${MANTICORE_STANDALONE:-0}" != "1" ]]; then
    DEFAULT_REPO_URL="https://repo.manticoresearch.com/repository/install"
    REPO_URL="${MANTICORE_INSTALLER_REPO_URL:-$DEFAULT_REPO_URL}"
    TEMP_DIR=$(mktemp -d /tmp/manticore-upgrade.XXXXXX)

    cleanup() {
        rm -rf "$TEMP_DIR"
    }

    trap cleanup EXIT

    download_bootstrap() {
        local url=$1
        local dest=$2

        if command -v curl >/dev/null 2>&1; then
            curl -fsSL "$url" -o "$dest"
        elif command -v wget >/dev/null 2>&1; then
            wget -qO "$dest" "$url"
        else
            echo "Neither curl nor wget was found on this system." >&2
            exit 1
        fi
    }

    download_bootstrap "${REPO_URL}/bootstrap.sh" "${TEMP_DIR}/bootstrap.sh"
    exec bash "${TEMP_DIR}/bootstrap.sh" --upgrade "$@"
fi

SCRIPT_DIR=$(dirname -- "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(cd "$SCRIPT_DIR" && pwd)
source "$SCRIPT_DIR/constants.sh"
source "$SCRIPT_DIR/detect.sh"
source "$SCRIPT_DIR/ui.sh"

REQUESTED_VERSION="${1:-}"
MANTICORE_START_SERVICE="${MANTICORE_START_SERVICE:-true}"
BACKUP_DATA="${MANTICORE_BACKUP_DATA:-${2:-no}}"
CURRENT_BACKUP_PATH=""

upgrade_repo_package() {
    if [[ "$OS_FAMILY" == "brew" ]]; then
        print_info "Homebrew installation does not require the Manticore repository package."
        return 0
    fi

    print_step "Refreshing Repository Bootstrap Package"

    if [[ "$OS_FAMILY" == "debian" ]]; then
        local deb_path
        deb_path=$(mktemp /tmp/manticore-repo.XXXXXX.deb)
        download_with_available_tool "$DEB_REPO_PACKAGE_URL" "$deb_path"
        install_debian_repo_package_file "$deb_path"
        rm -f "$deb_path"
    else
        if command -v dnf >/dev/null 2>&1; then
            sudo_exec dnf install -y "$RPM_REPO_PACKAGE_URL"
        else
            sudo_exec yum install -y "$RPM_REPO_PACKAGE_URL"
        fi
    fi
}

ensure_backup() {
    if [[ -d "$CURRENT_BACKUP_PATH" ]]; then
        print_info "Backup already exists at ${CURRENT_BACKUP_PATH}."
        return 0
    fi

    print_step "Backing Up Existing Installation"
    sudo_exec mkdir -p "$CURRENT_BACKUP_PATH"

    if [[ -d "$CONF_DIR" ]]; then
        sudo_exec cp -a "$CONF_DIR" "$CURRENT_BACKUP_PATH/"
    fi

    if [[ -d "$DATA_DIR" ]]; then
        if [[ "$BACKUP_DATA" == "yes" ]]; then
            ensure_service_stopped
            sudo_exec cp -a "$DATA_DIR" "$CURRENT_BACKUP_PATH/"
        else
            print_info "Data directory was not backed up. Rerun with --backup-data if you want a data backup before upgrading."
        fi
    fi

    print_success "Backup created at ${CURRENT_BACKUP_PATH}"
}

ensure_service_stopped() {
    if service_is_active; then
        print_step "Stopping Manticore Service"
        stop_service
    fi
}

upgrade_package() {
    local target_version=${1:-}
    local package_version=""
    local package_specs=()

    if [[ -n "$target_version" && "$OS_FAMILY" != "brew" ]]; then
        package_version=$(resolve_requested_version "$target_version") || return 1
        ensure_versioned_package_set_available "$package_version" || return 1
    elif [[ "$OS_FAMILY" == "debian" ]]; then
        package_version=$(get_latest_available_version || true)
        if [[ -n "$package_version" ]]; then
            ensure_versioned_package_set_available "$package_version" || return 1
        fi
    fi

    print_step "Upgrading Manticore"

    if [[ "$OS_FAMILY" == "debian" ]]; then
        if [[ -n "$package_version" ]]; then
            mapfile -t package_specs < <(versioned_debian_package_specs "$package_version")
            apt_get_install_with_repo_repair -y --allow-downgrades "${package_specs[@]}"
            mark_debian_split_packages_auto_for_thin_version "$package_version"
        else
            apt_get_install_with_repo_repair -y --only-upgrade "$PACKAGE_NAME"
        fi
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        if command -v dnf >/dev/null 2>&1; then
            if [[ -n "$package_version" ]]; then
                prepare_rpm_packages_for_version "$package_version"
                mapfile -t package_specs < <(versioned_rpm_package_specs "$package_version")
                sudo_exec dnf install -y "${package_specs[@]}"
            else
                sudo_exec dnf upgrade -y "$PACKAGE_NAME"
            fi
        else
            if [[ -n "$package_version" ]]; then
                prepare_rpm_packages_for_version "$package_version"
                mapfile -t package_specs < <(versioned_rpm_package_specs "$package_version")
                sudo_exec yum install -y "${package_specs[@]}"
            else
                sudo_exec yum update -y "$PACKAGE_NAME"
            fi
        fi
    elif [[ "$OS_FAMILY" == "brew" ]]; then
        if [[ -n "$target_version" ]]; then
            print_error "--version is not supported for Homebrew upgrades."
            return 1
        fi
        brew upgrade "$BREW_SERVICE_NAME" || brew install "$BREW_PACKAGE_NAME"
    fi
}


ensure_service_started() {
    if [[ "$MANTICORE_START_SERVICE" != "true" ]]; then
        print_info "Skipping service start because --no-start was requested."
        return 0
    fi

    if service_is_active; then
        print_info "Service is already running."
        return 0
    fi

    ensure_default_port_available
    start_service
}

verify_upgrade() {
    print_step "Verification"

    if ! command -v searchd >/dev/null 2>&1; then
        print_error "Binary 'searchd' not found after upgrade."
        return 1
    fi

    if service_is_active; then
        print_success "Upgrade successful. Service is running."
    else
        print_warn "Upgrade finished but service is not active."
    fi
}

upgrade_flow() {
    local requested_version=${1:-$REQUESTED_VERSION}
    local installed_version backup_suffix

    if ! package_installed; then
        print_error "Manticore is not installed."
        exit 1
    fi

    installed_version=$(get_installed_version)
    backup_suffix=$(echo "${installed_version:-unknown}" | tr '/: ' '___')
    CURRENT_BACKUP_PATH="${MANTICORE_BACKUP_DIR:-$BACKUP_DIR}/manticore_backup_${backup_suffix}"

    ensure_backup
    upgrade_repo_package
    refresh_package_metadata
    upgrade_package "$requested_version"
    ensure_service_started
    verify_upgrade
}

upgrade_main() {
    detect_os
    detect_arch

    if [[ "$OS_FAMILY" == "unknown" ]]; then
        print_unsupported_os
        exit 1
    fi

    upgrade_flow "$REQUESTED_VERSION"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" && "${MANTICORE_STANDALONE:-0}" != "1" ]]; then
    upgrade_main "$@"
fi
