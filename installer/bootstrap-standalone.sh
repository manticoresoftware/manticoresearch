#!/bin/bash

set -euo pipefail

MANTICORE_STANDALONE=1
SCRIPT_DIR=$(dirname -- "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(cd "$SCRIPT_DIR" && pwd)

# ---- constants.sh ----
PACKAGE_NAME="manticore"
DEB_FALLBACK_VERSIONED_PACKAGE_NAMES="manticore manticore-server manticore-server-core manticore-tools manticore-dev manticore-common"
RPM_FALLBACK_VERSIONED_PACKAGE_NAMES="manticore manticore-server manticore-server-core manticore-tools manticore-devel manticore-common"
SERVICE_NAME="manticore"
BREW_PACKAGE_NAME="manticoresoftware/tap/manticore"
BREW_SERVICE_NAME="manticore"

DEB_REPO_PACKAGE_NAME="manticore-repo"
RPM_REPO_PACKAGE_NAME="manticore-repo"

DEB_REPO_PACKAGE_URL="https://repo.manticoresearch.com/manticore-repo.noarch.deb"
RPM_REPO_PACKAGE_URL="https://repo.manticoresearch.com/manticore-repo.noarch.rpm"

# Keep in sync with listen directives in ../manticore.conf.in.
DEFAULT_PORTS="9306 9308 9312"

CONF_DIR="/etc/manticore"
DATA_DIR="/var/lib/manticore"
LOG_DIR="/var/log/manticore"

BACKUP_DIR="/var/backups/manticore"

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

# ---- ui.sh ----

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

use_color() {
    [[ -t 1 && "${SILENT:-false}" != "true" ]]
}

emit_line() {
    local plain_tag=$1
    local color=$2
    local symbol=$3
    local message=$4

    if use_color; then
        echo -e "${color}${symbol}${NC} ${message}"
    else
        echo "[${plain_tag}] ${message}"
    fi
}

log_info() { emit_line "DETECT" "$GREEN" "[DETECT]" "$1"; }
log_warn() { emit_line "DETECT" "$YELLOW" "[DETECT]" "$1"; }
log_err()  { emit_line "DETECT" "$RED" "[DETECT]" "$1"; }

print_success() {
    emit_line "SUCCESS" "$GREEN" "✔ ${BOLD}[SUCCESS]" "$1"
}

print_error() {
    if use_color; then
        echo -e "${RED}✘ ${BOLD}[ERROR]${NC} $1" >&2
    else
        echo "[ERROR] $1" >&2
    fi
}

print_warn() {
    emit_line "WARNING" "$YELLOW" "⚠ ${BOLD}[WARNING]" "$1"
}

print_info() {
    emit_line "INFO" "$CYAN" "ℹ ${BOLD}[INFO]" "$1"
}

print_step() {
    if use_color; then
        echo -e "\n${BLUE}======================================================================"
        echo -e "  ${BOLD}STEP: $1${NC}"
        echo -e "${BLUE}======================================================================${NC}"
    else
        echo ""
        echo "[STEP] $1"
    fi
}

ask_confirm() {
    if [[ "${SILENT:-false}" == "true" ]]; then
        return 0
    fi

    if [[ ! -t 0 ]]; then
        return 1
    fi

    local prompt="$1"
    local response

    while true; do
        read -p "$(echo -e "${YELLOW}[?] ${NC}${prompt} [y/N] ")" -n 1 response
        echo ""

        case "$response" in
            [yY][eE][sS]|[yY])
                return 0
                ;;
            [nN][oO]|[nN]|"")
                return 1
                ;;
            *)
                echo -e "${RED}  Please answer with 'y' or 'n'.${NC}"
                ;;
        esac
    done
}

sudo_exec() {
    if [[ $# -eq 0 ]]; then
        print_error "sudo_exec requires a command"
        return 1
    fi

    if [[ $EUID -eq 0 ]]; then
        "$@"
    else
        print_info "Running with sudo: $*"
        sudo "$@"
    fi
}

if [[ "${BASH_SOURCE[0]}" == "${0}" && "${MANTICORE_STANDALONE:-0}" != "1" ]]; then
    print_step "Testing UI Components"
    print_info "This is an info message."
    print_success "This is a success message."
    print_warn "This is a warning message."
    print_error "This is an error message."
fi

# ---- upgrade.sh ----


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

# ---- install.sh ----



SILENT=false
REQUESTED_VERSION="${1:-}"
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
    if [[ "$OS_FAMILY" == "brew" ]]; then
        print_info "Homebrew installation does not require the Manticore repository package."
        return 0
    fi

    if repo_package_installed; then
        print_info "Repository bootstrap package already installed."
        return 0
    fi

    print_step "Installing Repository Bootstrap Package"

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

    print_success "Repository bootstrap package installed."
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
                sudo_exec dnf install -y "${package_specs[@]}"
            else
                sudo_exec dnf install -y "$PACKAGE_NAME"
            fi
        else
            if [[ -n "$package_version" ]]; then
                prepare_rpm_packages_for_version "$package_version"
                mapfile -t package_specs < <(versioned_rpm_package_specs "$package_version")
                sudo_exec yum install -y "${package_specs[@]}"
            else
                sudo_exec yum install -y "$PACKAGE_NAME"
            fi
        fi
    elif [[ "$OS_FAMILY" == "brew" ]]; then
        if [[ -n "$target_version" ]]; then
            print_error "--version is not supported for Homebrew installs."
            return 1
        fi
        brew install "$BREW_PACKAGE_NAME"
    fi

    print_success "Manticore package is installed."
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
    else
        print_warn "Manticore package is installed but service is not active."
    fi
}

install_flow() {
    local requested_version=${1:-$REQUESTED_VERSION}

    warn_about_manual_installation
    install_repo_package
    refresh_package_metadata
    install_manticore_package "$requested_version"
    ensure_service_started
    verify_installation
}

install_main() {
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

# ---- uninstall.sh ----



ACTION_MODE="${1:-uninstall}"

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

    if [[ ! -t 0 ]]; then
        print_error "--purge-all removes $CONF_DIR and $DATA_DIR. Rerun with --silent to confirm this non-interactively."
        return 1
    fi

    local response
    echo "This will remove Manticore configuration and data directories:"
    echo "  $CONF_DIR"
    echo "  $DATA_DIR"
    read -r -p "Type DELETE to continue: " response

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

# ---- detect.sh ----


detect_os() {
    OS_ID="unknown"
    OS_VERSION="unknown"
    OS_LIKE=""
    OS_FAMILY="unknown"

    if [[ -f /etc/os-release ]]; then
        . /etc/os-release
        OS_ID=${ID:-unknown}
        OS_VERSION=${VERSION_ID:-unknown}
        OS_LIKE=${ID_LIKE:-}

        if [[ "$OS_ID" == "debian" || "$OS_ID" == "ubuntu" || "$OS_ID" == "linuxmint" || "$OS_LIKE" == *debian* ]]; then
            OS_FAMILY="debian"
        elif [[ "$OS_ID" == "rhel" || "$OS_ID" == "centos" || "$OS_ID" == "fedora" || "$OS_ID" == "rocky" || "$OS_ID" == "almalinux" || "$OS_ID" == "amzn" || "$OS_LIKE" == *rhel* || "$OS_LIKE" == *fedora* ]]; then
            OS_FAMILY="rpm"
        fi
    else
        local kernel
        kernel=$(uname -s 2>/dev/null || echo unknown)
        case "$kernel" in
            Darwin)
                OS_ID="macos"
                if command -v sw_vers >/dev/null 2>&1; then
                    OS_VERSION=$(sw_vers -productVersion)
                fi
                ;;
            *)
                OS_ID="$kernel"
                ;;
        esac
    fi

    if command -v brew >/dev/null 2>&1 && ! command -v apt-get >/dev/null 2>&1 && ! command -v yum >/dev/null 2>&1 && ! command -v dnf >/dev/null 2>&1; then
        OS_FAMILY="brew"
    fi

    log_info "OS: $OS_ID $OS_VERSION ($OS_FAMILY)"
}

detect_arch() {
    local arch_raw
    arch_raw=$(uname -m)

    case "$arch_raw" in
        x86_64|amd64)
            MANTICORE_ARCH="amd64"
            ;;
        aarch64|arm64)
            MANTICORE_ARCH="arm64"
            ;;
        *)
            MANTICORE_ARCH="unsupported"
            ;;
    esac

    log_info "Architecture: $MANTICORE_ARCH"
}

is_supported_arch() {
    [[ "$MANTICORE_ARCH" != "unsupported" ]]
}

print_unsupported_os() {
    local install_url="https://manticoresearch.com/install/"
    local docker_url="https://github.com/manticoresoftware/docker#production-use"
    local os_label="$OS_ID"

    case "$OS_ID" in
        macos)
            os_label="macOS"
            ;;
        unknown|"")
            os_label="this OS"
            ;;
        *)
            if [[ "$OS_VERSION" != "unknown" ]]; then
                os_label="$OS_ID $OS_VERSION"
            fi
            ;;
    esac

    print_error "${os_label} is not supported by this installer. See ${install_url} for supported installation methods."
    if command -v docker >/dev/null 2>&1; then
        print_info "Docker is available on this host. Manticore can also be run with Docker; see ${docker_url}."
    fi
}

is_interactive_session() {
    [[ -t 0 && -t 1 ]]
}


searchd_owned_by_package() {
    local searchd_path
    searchd_path=$(command -v searchd 2>/dev/null || true)

    if [[ -z "$searchd_path" ]]; then
        return 1
    fi

    if [[ "$OS_FAMILY" == "debian" ]]; then
        dpkg -S "$searchd_path" 2>/dev/null | grep -q '^manticore'
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        rpm -qf "$searchd_path" 2>/dev/null | grep -q '^manticore'
    elif [[ "$OS_FAMILY" == "brew" ]]; then
        brew list --formula "$BREW_SERVICE_NAME" >/dev/null 2>&1
    else
        return 1
    fi
}

warn_about_manual_installation() {
    if package_installed || ! command -v searchd >/dev/null 2>&1 || searchd_owned_by_package; then
        return 0
    fi

    print_warn "Found an existing searchd binary outside the Manticore package manager install."
    print_warn "Continuing may leave multiple Manticore installations on this host."
}

port_in_use() {
    local port=$1

    if command -v ss >/dev/null 2>&1; then
        ss -ltn 2>/dev/null | awk '{print $4}' | grep -Eq "(^|:)${port}$"
    elif command -v netstat >/dev/null 2>&1; then
        netstat -ltn 2>/dev/null | awk '{print $4}' | grep -Eq "(^|:)${port}$"
    else
        return 1
    fi
}

ensure_default_port_available() {
    local port

    if service_is_active; then
        return 0
    fi

    for port in $DEFAULT_PORTS; do
        if port_in_use "$port"; then
            print_error "Port ${port} is already in use. Stop the conflicting service or adjust /etc/manticoresearch/manticore.conf before starting Manticore."
            return 1
        fi
    done
}

repo_package_installed() {
    if [[ "$OS_FAMILY" == "debian" ]]; then
        dpkg-query -W -f='${Status}\n' "$DEB_REPO_PACKAGE_NAME" 2>/dev/null | grep -q "install ok installed"
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        rpm -q "$RPM_REPO_PACKAGE_NAME" >/dev/null 2>&1
    elif [[ "$OS_FAMILY" == "brew" ]]; then
        return 0
    else
        return 1
    fi
}

download_with_available_tool() {
    local url=$1
    local dest=$2

    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$url" -o "$dest"
    elif command -v wget >/dev/null 2>&1; then
        wget -qO "$dest" "$url"
    else
        print_error "Neither curl nor wget was found on this system."
        return 1
    fi
}

install_debian_repo_package_file() {
    local deb_path=$1
    local force=${2:-no}
    local downloaded_package downloaded_version installed_version

    downloaded_package=$(dpkg-deb -f "$deb_path" Package 2>/dev/null || true)
    downloaded_version=$(dpkg-deb -f "$deb_path" Version 2>/dev/null || true)

    if [[ "$downloaded_package" != "$DEB_REPO_PACKAGE_NAME" || -z "$downloaded_version" ]]; then
        print_error "Downloaded repository package is not a valid ${DEB_REPO_PACKAGE_NAME} package."
        return 1
    fi

    installed_version=$(dpkg-query -W -f='${Version}\n' "$DEB_REPO_PACKAGE_NAME" 2>/dev/null || true)
    if [[ "$force" != "force" && -n "$installed_version" && "$installed_version" == "$downloaded_version" ]]; then
        print_info "Repository bootstrap package ${DEB_REPO_PACKAGE_NAME} ${installed_version} is already installed."
        return 0
    fi

    sudo_exec dpkg -i "$deb_path"
}

delete_file_if_present() {
    local path=$1
    [[ -n "$path" && -f "$path" ]] && rm -f "$path"
}

repair_debian_repo_package() {
    local deb_path

    [[ "$OS_FAMILY" == "debian" ]] || return 0

    print_step "Repairing Repository Bootstrap Package"
    deb_path=$(mktemp /tmp/manticore-repo.XXXXXX.deb)
    download_with_available_tool "$DEB_REPO_PACKAGE_URL" "$deb_path"
    install_debian_repo_package_file "$deb_path" force
    delete_file_if_present "$deb_path"
}

apt_output_indicates_repo_auth_error() {
    local output_file=$1

    grep -Eiq 'NO_PUBKEY|EXPKEYSIG|BADSIG|GPG error|invalid signature|The following signatures were invalid|signatures couldn.t be verified|public key is not available|repository .* is not signed|InRelease is not signed|Release file is not signed|unauthenticated packages|cannot be authenticated|There is no public key available' "$output_file"
}

run_with_output_capture() {
    local output_file=$1
    shift

    set +e
    "$@" 2>&1 | tee "$output_file"
    local status=${PIPESTATUS[0]}
    set -e
    return "$status"
}

refresh_debian_package_metadata_once() {
    sudo_exec apt-get update
}

refresh_debian_package_metadata() {
    local output_file

    output_file=$(mktemp /tmp/manticore-apt-update.XXXXXX.log)
    if run_with_output_capture "$output_file" refresh_debian_package_metadata_once; then
        if apt_output_indicates_repo_auth_error "$output_file"; then
            print_warn "Apt repository authentication warning detected. Reinstalling the repository bootstrap package and retrying metadata refresh."
            repair_debian_repo_package
            delete_file_if_present "$output_file"
            sudo_exec apt-get update
            return $?
        fi
        delete_file_if_present "$output_file"
        return 0
    fi

    if apt_output_indicates_repo_auth_error "$output_file"; then
        print_warn "Apt repository authentication failed. Reinstalling the repository bootstrap package and retrying metadata refresh."
        repair_debian_repo_package
        delete_file_if_present "$output_file"
        sudo_exec apt-get update
        return $?
    fi

    delete_file_if_present "$output_file"
    return 1
}

apt_get_install_with_repo_repair() {
    local output_file

    output_file=$(mktemp /tmp/manticore-apt-install.XXXXXX.log)
    if run_with_output_capture "$output_file" sudo_exec apt-get install "$@"; then
        delete_file_if_present "$output_file"
        return 0
    fi

    if apt_output_indicates_repo_auth_error "$output_file"; then
        print_warn "Apt package authentication failed. Reinstalling the repository bootstrap package and retrying install."
        repair_debian_repo_package
        sudo_exec apt-get update
        delete_file_if_present "$output_file"
        sudo_exec apt-get install "$@"
        return $?
    fi

    delete_file_if_present "$output_file"
    return 1
}

ensure_repo_package_installed() {
    if [[ "$OS_FAMILY" == "brew" ]]; then
        return 0
    fi

    if repo_package_installed; then
        print_info "Repository bootstrap package already installed."
        return 0
    fi

    print_step "Installing Repository Bootstrap Package"

    if [[ "$OS_FAMILY" == "debian" ]]; then
        local deb_path
        deb_path=$(mktemp /tmp/manticore-repo.XXXXXX.deb)
        download_with_available_tool "$DEB_REPO_PACKAGE_URL" "$deb_path"
        install_debian_repo_package_file "$deb_path"
        rm -f "$deb_path"
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        if command -v dnf >/dev/null 2>&1; then
            sudo_exec dnf install -y "$RPM_REPO_PACKAGE_URL"
        else
            sudo_exec yum install -y "$RPM_REPO_PACKAGE_URL"
        fi
    fi

    print_success "Repository bootstrap package installed."
}

package_installed() {
    local package

    if [[ "$OS_FAMILY" == "debian" ]]; then
        for package in $DEB_FALLBACK_VERSIONED_PACKAGE_NAMES; do
            dpkg-query -W -f='${Status}\n' "$package" 2>/dev/null | grep -q "install ok installed" && return 0
        done
        return 1
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        for package in $RPM_FALLBACK_VERSIONED_PACKAGE_NAMES; do
            rpm -q "$package" >/dev/null 2>&1 && return 0
        done
        return 1
    elif [[ "$OS_FAMILY" == "brew" ]]; then
        brew list --formula "$BREW_SERVICE_NAME" >/dev/null 2>&1
    else
        return 1
    fi
}

get_installed_version() {
    local package

    if ! package_installed; then
        return 1
    fi

    if [[ "$OS_FAMILY" == "debian" ]]; then
        for package in $DEB_FALLBACK_VERSIONED_PACKAGE_NAMES; do
            dpkg-query -W -f='${Status} ${Version}\n' "$package" 2>/dev/null | awk '$1 == "install" && $2 == "ok" && $3 == "installed" {print $4; found=1} END {exit found ? 0 : 1}' && return 0
        done
        return 1
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        for package in $RPM_FALLBACK_VERSIONED_PACKAGE_NAMES; do
            rpm -q --qf '%{VERSION}-%{RELEASE}\n' "$package" 2>/dev/null && return 0
        done
        return 1
    elif [[ "$OS_FAMILY" == "brew" ]]; then
        brew list --versions "$BREW_SERVICE_NAME" 2>/dev/null | awk 'NR==1 {print $2}'
    fi
}

get_latest_available_version() {
    if [[ "$OS_FAMILY" == "debian" ]]; then
        apt-cache madison "$PACKAGE_NAME" 2>/dev/null | awk 'NR==1 {print $3}'
        return 0
    fi

    if [[ "$OS_FAMILY" == "rpm" ]]; then
        if command -v dnf >/dev/null 2>&1; then
            dnf --showduplicates list "$PACKAGE_NAME" 2>/dev/null | awk -v pkg="$PACKAGE_NAME" '$1 ~ ("^" pkg "[.]") {print $2}' | tail -n 1
        else
            yum --showduplicates list "$PACKAGE_NAME" 2>/dev/null | awk -v pkg="$PACKAGE_NAME" '$1 ~ ("^" pkg "[.]") {print $2}' | tail -n 1
        fi
        return 0
    fi

    if [[ "$OS_FAMILY" == "brew" ]]; then
        brew info --formula "$BREW_SERVICE_NAME" 2>/dev/null | awk '/^==> / {for (i=1; i<=NF; i++) if ($i == "stable") {print $(i+1); exit}}'
    fi
}

normalize_version() {
    local version=$1
    version=${version#*:}
    version=${version%%+*}
    echo "$version"
}

version_matches_request() {
    local available=$1
    local requested=$2
    local normalized_available normalized_requested

    normalized_available=$(normalize_version "$available")
    normalized_requested=$(normalize_version "$requested")

    [[ "$available" == "$requested" ]] && return 0
    [[ "$normalized_available" == "$normalized_requested" ]] && return 0
    [[ "$normalized_available" == "$normalized_requested"[-_+~.]* ]] && return 0
    return 1
}

list_available_versions() {
    if [[ "$OS_FAMILY" == "debian" ]]; then
        apt-cache madison "$PACKAGE_NAME" 2>/dev/null | awk '{print $3}'
        return 0
    fi

    if [[ "$OS_FAMILY" == "rpm" ]]; then
        if command -v dnf >/dev/null 2>&1; then
            dnf --showduplicates list "$PACKAGE_NAME" 2>/dev/null | awk -v pkg="$PACKAGE_NAME" '$1 ~ ("^" pkg "[.]") {print $2}'
        else
            yum --showduplicates list "$PACKAGE_NAME" 2>/dev/null | awk -v pkg="$PACKAGE_NAME" '$1 ~ ("^" pkg "[.]") {print $2}'
        fi
        return 0
    fi

    if [[ "$OS_FAMILY" == "brew" ]]; then
        get_latest_available_version
        return 0
    fi

    return 1
}

resolve_requested_version() {
    local requested=$1
    local available matches resolved

    [[ -z "$requested" ]] && return 0

    matches=$(
        while IFS= read -r available; do
            if version_matches_request "$available" "$requested"; then
                printf '%s\n' "$available"
            fi
        done < <(list_available_versions)
    )

    if [[ -z "$matches" ]]; then
        print_error "Requested version $requested was not found in the configured Manticore repository."
        return 1
    fi

    resolved=$(printf '%s\n' "$matches" | sort -V | tail -n 1)
    if [[ "$resolved" != "$requested" ]]; then
        print_info "Resolved requested version $requested to package version $resolved." >&2
    fi
    printf '%s\n' "$resolved"
}

debian_manticore_package_is_fat() {
    local version=$1

    apt-cache show "${PACKAGE_NAME}=${version}" 2>/dev/null | awk '
        /^Breaks:/ || /^Provides:/ { field = $0; capture = 1; next }
        capture && /^[[:space:]]/ { field = field " " $0; next }
        capture { capture = 0 }
        END {
            exit field ~ /manticore-(common|server-core|server|tools|dev)/ ? 0 : 1
        }
    '
}

debian_dependency_fields_for_package_version() {
    local package=$1
    local version=$2

    apt-cache show "${package}=${version}" 2>/dev/null | awk '
        /^Depends:/ {
            line = $0
            sub(/^Depends:[[:space:]]*/, "", line)
            capture = 1
            next
        }
        capture && /^[[:space:]]/ {
            line = line " " $0
            next
        }
        capture {
            print line
            capture = 0
        }
        END {
            if (capture) print line
        }
    '
}

debian_same_version_dependency_package_names() {
    local package=$1
    local version=$2

    debian_dependency_fields_for_package_version "$package" "$version" | tr ',' '\n' | awk -v version="$version" '
        {
            gsub(/^[[:space:]]+|[[:space:]]+$/, "")
            if ($0 !~ /^manticore-/) next
            name = $0
            cut = length(name) + 1
            space_pos = index(name, " ")
            tab_pos = index(name, "	")
            paren_pos = index(name, "(")
            if (space_pos > 0 && space_pos < cut) cut = space_pos
            if (tab_pos > 0 && tab_pos < cut) cut = tab_pos
            if (paren_pos > 0 && paren_pos < cut) cut = paren_pos
            name = substr(name, 1, cut - 1)
            constraint = ""
            if (match($0, /\([^)]+\)/)) {
                constraint = substr($0, RSTART + 1, RLENGTH - 2)
                gsub(/^[[:space:]]+|[[:space:]]+$/, "", constraint)
                gsub(/[[:space:]]+/, " ", constraint)
            }
            if (constraint == "= " version) print name
        }
    '
}

versioned_debian_package_names() {
    local version=$1
    local package dep seen_key
    local queue=("$PACKAGE_NAME")
    local seen=" "
    local result=()

    while [[ ${#queue[@]} -gt 0 ]]; do
        package=${queue[0]}
        queue=("${queue[@]:1}")
        seen_key=" ${package} "
        [[ "$seen" == *"$seen_key"* ]] && continue

        seen+="${package} "
        result+=("$package")

        while IFS= read -r dep; do
            [[ -z "$dep" ]] && continue
            [[ "$seen" == *" ${dep} "* ]] && continue
            queue+=("$dep")
        done < <(debian_same_version_dependency_package_names "$package" "$version")
    done

    printf '%s\n' "${result[@]}"
}

versioned_debian_package_specs() {
    local version=$1
    local package

    if debian_manticore_package_is_fat "$version"; then
        printf '%s=%s\n' "$PACKAGE_NAME" "$version"
        return 0
    fi

    while IFS= read -r package; do
        [[ -z "$package" ]] && continue
        printf '%s=%s\n' "$package" "$version"
    done < <(versioned_debian_package_names "$version")

    versioned_debian_dependency_package_specs "$version"
}

mark_debian_split_packages_auto_for_thin_version() {
    local version=$1
    local package dep_package constraint
    local auto_packages=()

    [[ "$OS_FAMILY" == "debian" && -n "$version" ]] || return 0

    if debian_manticore_package_is_fat "$version"; then
        return 0
    fi

    while IFS= read -r package; do
        [[ -z "$package" || "$package" == "$PACKAGE_NAME" ]] && continue
        if dpkg-query -W -f='${Status}\n' "$package" 2>/dev/null | grep -q "install ok installed"; then
            auto_packages+=("$package")
        fi
    done < <(versioned_debian_package_names "$version")

    while IFS= read -r package; do
        [[ -z "$package" ]] && continue
        while IFS=$'\t' read -r dep_package constraint; do
            [[ -z "$dep_package" ]] && continue
            debian_package_in_versioned_set "$dep_package" "$version" && continue
            if dpkg-query -W -f='${Status}\n' "$dep_package" 2>/dev/null | grep -q "install ok installed"; then
                if [[ " ${auto_packages[*]} " != *" ${dep_package} "* ]]; then
                    auto_packages+=("$dep_package")
                fi
            fi
        done < <(debian_dependency_constraints_for_package_version "$package" "$version")
    done < <(versioned_debian_package_names "$version")

    if [[ ${#auto_packages[@]} -eq 0 ]]; then
        return 0
    fi

    sudo_exec apt-mark auto "${auto_packages[@]}" >/dev/null
    print_info "Marked Debian dependency packages as automatically installed: ${auto_packages[*]}"
}

debian_package_in_versioned_set() {
    local candidate=$1
    local version=${2:-}
    local package

    if [[ -n "$version" ]]; then
        while IFS= read -r package; do
            [[ "$candidate" == "$package" ]] && return 0
        done < <(versioned_debian_package_names "$version")
        return 1
    fi

    for package in $DEB_FALLBACK_VERSIONED_PACKAGE_NAMES; do
        [[ "$candidate" == "$package" ]] && return 0
    done

    [[ "$candidate" == "$PACKAGE_NAME" ]]
}

debian_dependency_constraints_for_package_version() {
    local package=$1
    local version=$2

    debian_dependency_fields_for_package_version "$package" "$version" | tr ',' '\n' | awk '
        {
            gsub(/^[[:space:]]+|[[:space:]]+$/, "")
            if ($0 !~ /^manticore-/) next
            name = $0
            cut = length(name) + 1
            space_pos = index(name, " ")
            tab_pos = index(name, "	")
            paren_pos = index(name, "(")
            if (space_pos > 0 && space_pos < cut) cut = space_pos
            if (tab_pos > 0 && tab_pos < cut) cut = tab_pos
            if (paren_pos > 0 && paren_pos < cut) cut = paren_pos
            name = substr(name, 1, cut - 1)
            constraint = ""
            if (match($0, /\([^)]+\)/)) {
                constraint = substr($0, RSTART + 1, RLENGTH - 2)
            }
            print name "\t" constraint
        }
    '
}

debian_version_satisfies_constraint() {
    local version=$1
    local constraint=$2
    local op constraint_version

    [[ -z "$constraint" ]] && return 0

    op=${constraint%% *}
    constraint_version=${constraint#* }
    [[ -z "$op" || -z "$constraint_version" || "$op" == "$constraint_version" ]] && return 0

    dpkg --compare-versions "$version" "$op" "$constraint_version"
}

debian_highest_available_version_satisfying_constraints() {
    local package=$1
    local constraints=$2
    local candidate constraint ok

    while IFS= read -r candidate; do
        [[ -z "$candidate" ]] && continue
        ok=1
        while IFS= read -r constraint; do
            [[ -z "$constraint" ]] && continue
            if ! debian_version_satisfies_constraint "$candidate" "$constraint"; then
                ok=0
                break
            fi
        done <<< "$constraints"

        if [[ "$ok" -eq 1 ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done < <(apt-cache madison "$package" 2>/dev/null | awk '{print $3}' | sort -Vr)

    return 1
}

debian_version_satisfies_constraints() {
    local version=$1
    local constraints=$2
    local constraint

    [[ -z "$version" ]] && return 1

    while IFS= read -r constraint; do
        [[ -z "$constraint" ]] && continue
        debian_version_satisfies_constraint "$version" "$constraint" || return 1
    done <<< "$constraints"

    return 0
}

versioned_debian_dependency_package_specs() {
    local version=$1
    local source_package dep_package constraint
    local dep_packages=()
    local dep_constraints=""
    local package_constraints selected_version installed_version

    while IFS= read -r source_package; do
        [[ -z "$source_package" ]] && continue
        while IFS=$'\t' read -r dep_package constraint; do
            [[ -z "$dep_package" ]] && continue
            debian_package_in_versioned_set "$dep_package" "$version" && continue
            [[ -z "$constraint" ]] && continue

            if [[ " ${dep_packages[*]} " != *" ${dep_package} "* ]]; then
                dep_packages+=("$dep_package")
            fi
            dep_constraints+="${dep_package}"$'\t'"${constraint}"$'\n'
        done < <(debian_dependency_constraints_for_package_version "$source_package" "$version")
    done < <(versioned_debian_package_names "$version")

    for dep_package in "${dep_packages[@]}"; do
        package_constraints=$(printf '%s' "$dep_constraints" | awk -F '\t' -v pkg="$dep_package" '$1 == pkg {print $2}')
        installed_version=$(dpkg-query -W -f='${Version}\n' "$dep_package" 2>/dev/null || true)
        if debian_version_satisfies_constraints "$installed_version" "$package_constraints"; then
            continue
        fi

        selected_version=$(debian_highest_available_version_satisfying_constraints "$dep_package" "$package_constraints" || true)
        if [[ -z "$selected_version" ]]; then
            print_error "Required dependency ${dep_package} has no available version satisfying constraints for Manticore ${version}."
            return 1
        fi
        printf '%s=%s\n' "$dep_package" "$selected_version"
    done
}

rpm_repoquery_requires() {
    local package=$1
    local version=$2

    if command -v dnf >/dev/null 2>&1; then
        dnf -q repoquery --requires "${package}-${version}" 2>/dev/null
    elif command -v repoquery >/dev/null 2>&1; then
        repoquery --requires "${package}-${version}" 2>/dev/null
    elif command -v yum >/dev/null 2>&1; then
        yum -q repoquery --requires "${package}-${version}" 2>/dev/null
    else
        return 1
    fi
}

rpm_same_version_dependency_package_names() {
    local package=$1
    local version=$2

    rpm_repoquery_requires "$package" "$version" | awk -v version="$version" '
        $1 ~ /^manticore-/ && $2 == "=" && ($3 == version || $3 == "0:" version) { print $1 }
    '
}

versioned_rpm_package_names() {
    local version=$1
    local package dep seen_key
    local queue=("$PACKAGE_NAME")
    local seen=" "
    local result=()

    if ! rpm_repoquery_requires "$PACKAGE_NAME" "$version" >/dev/null; then
        printf '%s\n' $RPM_FALLBACK_VERSIONED_PACKAGE_NAMES
        return 0
    fi

    while [[ ${#queue[@]} -gt 0 ]]; do
        package=${queue[0]}
        queue=("${queue[@]:1}")
        seen_key=" ${package} "
        [[ "$seen" == *"$seen_key"* ]] && continue

        seen+="${package} "
        result+=("$package")

        while IFS= read -r dep; do
            [[ -z "$dep" ]] && continue
            [[ "$seen" == *" ${dep} "* ]] && continue
            queue+=("$dep")
        done < <(rpm_same_version_dependency_package_names "$package" "$version")
    done

    printf '%s\n' "${result[@]}"
}

versioned_rpm_package_specs() {
    local version=$1
    local package

    while IFS= read -r package; do
        [[ -z "$package" ]] && continue
        printf '%s-%s\n' "$package" "$version"
    done < <(versioned_rpm_package_names "$version")
}


rpm_versioned_package_set_is_fat() {
    local version=$1
    local package_names=()

    mapfile -t package_names < <(versioned_rpm_package_names "$version")
    [[ ${#package_names[@]} -eq 1 && "${package_names[0]}" == "$PACKAGE_NAME" ]]
}

rpm_installed_manticore_is_fat() {
    rpm -q "$PACKAGE_NAME" >/dev/null 2>&1 || return 1
    ! rpm -q --requires "$PACKAGE_NAME" 2>/dev/null | awk '
        $1 ~ /^manticore-(server|tools|devel|common)/ && $2 == "=" { found=1 }
        END { exit found ? 0 : 1 }
    '
}

rpm_installed_split_package_names() {
    local package

    for package in $RPM_FALLBACK_VERSIONED_PACKAGE_NAMES; do
        [[ "$package" == "$PACKAGE_NAME" ]] && continue
        if rpm -q "$package" >/dev/null 2>&1; then
            printf '%s\n' "$package"
        fi
    done
}

prepare_rpm_packages_for_version() {
    local version=$1
    local split_packages=()

    [[ "$OS_FAMILY" == "rpm" && -n "$version" ]] || return 0

    mapfile -t split_packages < <(rpm_installed_split_package_names)
    if [[ ${#split_packages[@]} -gt 0 ]]; then
        print_info "Removing installed RPM split packages before version switch: ${split_packages[*]}"
        sudo_exec rpm -e --noscripts --nodeps "${split_packages[@]}"
    fi

    if ! rpm_versioned_package_set_is_fat "$version" && rpm_installed_manticore_is_fat; then
        print_info "Removing installed RPM fat package before installing split packages: $PACKAGE_NAME"
        sudo_exec rpm -e --noscripts --nodeps "$PACKAGE_NAME"
    fi
}

package_version_available() {
    local package=$1
    local version=$2

    if [[ "$OS_FAMILY" == "debian" ]]; then
        apt-cache madison "$package" 2>/dev/null | awk -v version="$version" '$3 == version {found=1} END {exit found ? 0 : 1}'
        return $?
    fi

    if [[ "$OS_FAMILY" == "rpm" ]]; then
        if command -v dnf >/dev/null 2>&1; then
            dnf --showduplicates list "$package" 2>/dev/null | awk -v pkg="$package" -v version="$version" '$1 ~ ("^" pkg "[.]") && $2 == version {found=1} END {exit found ? 0 : 1}'
        else
            yum --showduplicates list "$package" 2>/dev/null | awk -v pkg="$package" -v version="$version" '$1 ~ ("^" pkg "[.]") && $2 == version {found=1} END {exit found ? 0 : 1}'
        fi
        return $?
    fi

    return 1
}

ensure_versioned_package_set_available() {
    local version=$1
    local package package_names missing=0

    if [[ "$OS_FAMILY" == "debian" ]]; then
        if debian_manticore_package_is_fat "$version"; then
            package_names="$PACKAGE_NAME"
        else
            package_names=$(versioned_debian_package_names "$version")
        fi
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        package_names=$(versioned_rpm_package_names "$version")
    else
        return 0
    fi

    for package in $package_names; do
        if ! package_version_available "$package" "$version"; then
            print_error "Required package ${package} version ${version} is not available in the configured Manticore repository."
            missing=1
        fi
    done

    if [[ "$OS_FAMILY" == "debian" ]] && ! debian_manticore_package_is_fat "$version"; then
        versioned_debian_dependency_package_specs "$version" >/dev/null || missing=1
    fi

    if [[ "$missing" -ne 0 ]]; then
        print_error "The requested Manticore version has an incomplete package set for this OS/repository. Try --list-versions or choose another version."
        return 1
    fi
}

version_gt() {
    local left right
    left=$(normalize_version "$1")
    right=$(normalize_version "$2")

    [[ "$(printf '%s\n%s\n' "$right" "$left" | sort -V | tail -n 1)" == "$left" && "$left" != "$right" ]]
}
service_is_active() {
    if [[ "$OS_FAMILY" == "brew" ]]; then
        brew services list 2>/dev/null | awk -v svc="$BREW_SERVICE_NAME" '$1 == svc && $2 == "started" {found=1} END {exit found ? 0 : 1}'
    elif systemctl_usable; then
        systemctl is-active --quiet "$SERVICE_NAME" >/dev/null 2>&1
    else
        ps aux | grep -q '[s]earchd'
    fi
}

systemctl_usable() {
    command -v systemctl >/dev/null 2>&1 && systemctl show >/dev/null 2>&1
}

start_service() {
    if [[ "$OS_FAMILY" == "brew" ]]; then
        brew services start "$BREW_SERVICE_NAME"
    elif systemctl_usable; then
        sudo_exec systemctl start "$SERVICE_NAME"
    else
        sudo_exec mkdir -p /var/run/manticore
        sudo_exec searchd
    fi
}

stop_service() {
    if [[ "$OS_FAMILY" == "brew" ]]; then
        brew services stop "$BREW_SERVICE_NAME"
    elif systemctl_usable; then
        sudo_exec systemctl stop "$SERVICE_NAME"
    else
        sudo_exec searchd --stopwait
    fi
}

desired_version_installed() {
    local desired=$1
    local installed

    if [[ -z "$desired" ]]; then
        package_installed
        return $?
    fi

    if ! package_installed; then
        return 1
    fi

    installed=$(get_installed_version)
    version_matches_request "$installed" "$desired"
}

refresh_package_metadata() {
    if [[ "$OS_FAMILY" == "debian" ]]; then
        refresh_debian_package_metadata
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        if command -v dnf >/dev/null 2>&1; then
            sudo_exec dnf makecache
        else
            sudo_exec yum makecache
        fi
    elif [[ "$OS_FAMILY" == "brew" ]]; then
        brew update
    fi
}

if [[ "${BASH_SOURCE[0]}" == "${0}" && "${MANTICORE_STANDALONE:-0}" != "1" ]]; then
    detect_os
    detect_arch
fi

# ---- main.sh ----



SILENT=false
SPECIFIC_VERSION=""
ACTION=""
UPGRADE_REQUESTED=false
MANTICORE_START_SERVICE=true
MANTICORE_BACKUP_DATA=no
MANTICORE_BACKUP_DIR=""
LIST_VERSIONS_OUTPUT_FILE=""

parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -s|--silent|-y|--yes)
                SILENT=true
                shift
                ;;
            --no-start)
                MANTICORE_START_SERVICE=false
                shift
                ;;
            --backup-data)
                MANTICORE_BACKUP_DATA=yes
                shift
                ;;
            --no-backup-data)
                MANTICORE_BACKUP_DATA=no
                shift
                ;;
            --backup-dir)
                if [[ -z "${2:-}" ]]; then
                    print_error "--backup-dir requires a value."
                    exit 1
                fi
                MANTICORE_BACKUP_DIR="$2"
                shift 2
                ;;
            -v|--version)
                if [[ -z "${2:-}" ]]; then
                    print_error "--version requires a value."
                    exit 1
                fi
                SPECIFIC_VERSION="$2"
                shift 2
                ;;
            -u|--uninstall)
                ACTION="uninstall"
                shift
                ;;
            --purge)
                ACTION="purge"
                shift
                ;;
            --purge-all)
                ACTION="purge-all"
                shift
                ;;
            --upgrade)
                UPGRADE_REQUESTED=true
                shift
                ;;
            --list-versions)
                ACTION="list-versions"
                if [[ -n "${2:-}" && "${2:0:1}" != "-" ]]; then
                    LIST_VERSIONS_OUTPUT_FILE="$2"
                    shift 2
                else
                    shift
                fi
                ;;
            --list-versions=*)
                ACTION="list-versions"
                LIST_VERSIONS_OUTPUT_FILE="${1#*=}"
                shift
                ;;
            --list-versions-file)
                if [[ -z "${2:-}" ]]; then
                    print_error "$1 requires a value."
                    exit 1
                fi
                ACTION="list-versions"
                LIST_VERSIONS_OUTPUT_FILE="$2"
                shift 2
                ;;
            --list-versions-file=*)
                ACTION="list-versions"
                LIST_VERSIONS_OUTPUT_FILE="${1#*=}"
                shift
                ;;
            *)
                print_warn "Ignoring unknown argument: $1"
                shift
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

    ensure_repo_package_installed
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

    if [[ "$UPGRADE_REQUESTED" == "true" && -z "$SPECIFIC_VERSION" ]]; then
        ACTION="upgrade"
        return 0
    fi

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

    if ! version_gt "$target_version" "$current_version"; then
        print_info "Installed version $current_version is already up to date."
        exit 0
    fi

    if [[ "$UPGRADE_REQUESTED" == "true" ]]; then
        ACTION="upgrade"
        return 0
    fi

    if [[ "$SILENT" == "true" ]]; then
        print_info "Newer version $target_version is available. Rerun with --upgrade to apply it."
        exit 0
    fi

    if ask_confirm "Version $target_version is available. Upgrade from $current_version?"; then
        ACTION="upgrade"
    else
        print_info "Leaving version $current_version installed."
        exit 0
    fi
}

execute_action() {
    if [[ "${MANTICORE_STANDALONE:-0}" == "1" ]]; then
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
        return 0
    fi

    case "$ACTION" in
        install)
            bash "$SCRIPT_DIR/install.sh" "$SPECIFIC_VERSION"
            ;;
        upgrade)
            MANTICORE_UPGRADE_MODULE=1 bash "$SCRIPT_DIR/upgrade.sh" "$SPECIFIC_VERSION"
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

main() {
    if [[ "${MANTICORE_STANDALONE:-0}" == "1" ]]; then
        print_log "Initializing self-contained Manticore Search Installer..."
        print_log "Logs are being written to $INSTALL_LOG"
    fi

    parse_args "$@"
    export SILENT MANTICORE_START_SERVICE MANTICORE_BACKUP_DATA MANTICORE_BACKUP_DIR
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
