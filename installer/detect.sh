#!/bin/bash

SCRIPT_DIR=$(dirname -- "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(cd "$SCRIPT_DIR" && pwd)
source "$SCRIPT_DIR/constants.sh"
source "$SCRIPT_DIR/ui.sh"

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
        brew_formula_installed_for_channel release || brew_formula_installed_for_channel dev
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
        rpm -q "$RPM_REPO_PACKAGE_NAME" >/dev/null 2>&1 || rpm -q "$RPM_DEV_REPO_PACKAGE_NAME" >/dev/null 2>&1
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
    local downloaded_package downloaded_version installed_version installed_status

    downloaded_package=$(dpkg-deb -f "$deb_path" Package 2>/dev/null || true)
    downloaded_version=$(dpkg-deb -f "$deb_path" Version 2>/dev/null || true)

    if [[ "$downloaded_package" != "$DEB_REPO_PACKAGE_NAME" || -z "$downloaded_version" ]]; then
        print_error "Downloaded repository package is not a valid ${DEB_REPO_PACKAGE_NAME} package."
        return 1
    fi

    installed_version=$(dpkg-query -W -f='${Version}' "$DEB_REPO_PACKAGE_NAME" 2>/dev/null || true)
    installed_status=$(dpkg-query -W -f='${Status}' "$DEB_REPO_PACKAGE_NAME" 2>/dev/null || true)
    if [[ "$force" != "force" && "$installed_status" == "install ok installed" && -n "$installed_version" && "$installed_version" == "$downloaded_version" ]]; then
        print_info "Repository bootstrap package ${DEB_REPO_PACKAGE_NAME} ${installed_version} is already installed."
        return 0
    fi

    if [[ -n "$installed_version" && "$installed_status" != "install ok installed" ]]; then
        print_warn "Repository bootstrap package ${DEB_REPO_PACKAGE_NAME} ${installed_version} is present but not fully configured. Reinstalling it with dependencies."
        sudo_exec apt-get install -f -y
        installed_version=$(dpkg-query -W -f='${Version}' "$DEB_REPO_PACKAGE_NAME" 2>/dev/null || true)
        installed_status=$(dpkg-query -W -f='${Status}' "$DEB_REPO_PACKAGE_NAME" 2>/dev/null || true)

        if [[ "$force" != "force" && "$installed_status" == "install ok installed" && -n "$installed_version" && "$installed_version" == "$downloaded_version" ]]; then
            return 0
        fi
    fi

    if [[ -z "$installed_version" ]]; then
        sudo_exec apt-get update
    fi
    sudo_exec apt-get install -y "$deb_path"
}

delete_file_if_present() {
    local path=$1
    [[ -n "$path" && -f "$path" ]] && rm -f "$path"
}

repair_debian_repo_package() {
    local deb_path channel repo_url

    [[ "$OS_FAMILY" == "debian" ]] || return 0

    channel="${MANTICORE_REPO_CHANNEL:-$(detect_debian_repo_channel || echo release)}"
    repo_url=$(deb_repo_package_url_for_channel "$channel") || return 1

    print_step "Repairing Repository Bootstrap Package"
    print_info "Reinstalling Manticore ${channel} repository package."
    deb_path=$(mktemp /tmp/manticore-repo.XXXXXX.deb)
    download_with_available_tool "$repo_url" "$deb_path"
    install_debian_repo_package_file "$deb_path" force
    delete_file_if_present "$deb_path"
}

deb_repo_package_url_for_channel() {
    case "$1" in
        release) echo "$DEB_RELEASE_REPO_PACKAGE_URL" ;;
        dev) echo "$DEB_DEV_REPO_PACKAGE_URL" ;;
        *) return 1 ;;
    esac
}

rpm_repo_package_url_for_channel() {
    case "$1" in
        release) echo "$RPM_RELEASE_REPO_PACKAGE_URL" ;;
        dev) echo "$RPM_DEV_REPO_PACKAGE_URL" ;;
        *) return 1 ;;
    esac
}

brew_package_name_for_channel() {
    case "$1" in
        release) echo "$BREW_RELEASE_PACKAGE_NAME" ;;
        dev) echo "$BREW_DEV_PACKAGE_NAME" ;;
        *) return 1 ;;
    esac
}

brew_service_name_for_channel() {
    case "$1" in
        release) echo "$BREW_RELEASE_SERVICE_NAME" ;;
        dev) echo "$BREW_DEV_SERVICE_NAME" ;;
        *) return 1 ;;
    esac
}

brew_formula_installed_for_channel() {
    local formula

    formula=$(brew_package_name_for_channel "$1") || return 1
    brew list --formula "$formula" >/dev/null 2>&1
}

brew_channel_or_default() {
    if [[ -n "${MANTICORE_REPO_CHANNEL:-}" ]]; then
        echo "$MANTICORE_REPO_CHANNEL"
        return 0
    fi

    if brew_formula_installed_for_channel dev && ! brew_formula_installed_for_channel release; then
        echo dev
        return 0
    fi

    echo release
}

brew_package_name() {
    local channel

    channel=$(brew_channel_or_default)
    brew_package_name_for_channel "$channel"
}

brew_service_name() {
    local channel

    channel=$(brew_channel_or_default)
    brew_service_name_for_channel "$channel"
}

rpm_run() {
    local command=$1
    shift

    "$command" "$@"
}

rpm_sudo_run() {
    local command=$1
    shift

    sudo_exec "$command" "$@"
}

installed_rpm_repo_package_names() {
    local package

    for package in "$RPM_REPO_PACKAGE_NAME" "$RPM_DEV_REPO_PACKAGE_NAME"; do
        if rpm -q "$package" >/dev/null 2>&1; then
            printf "%s\n" "$package"
        fi
    done
}

remove_rpm_repo_packages() {
    local package_names=()

    mapfile -t package_names < <(installed_rpm_repo_package_names)
    [[ ${#package_names[@]} -eq 0 ]] && return 0

    if command -v dnf >/dev/null 2>&1; then
        sudo_exec dnf remove -y "${package_names[@]}"
    elif command -v yum >/dev/null 2>&1; then
        sudo_exec yum remove -y "${package_names[@]}"
    else
        print_error "Neither dnf nor yum is available to remove RPM repository packages."
        return 1
    fi
}

install_rpm_repo_package_url() {
    local repo_url=$1

    if command -v dnf >/dev/null 2>&1; then
        sudo_exec dnf install -y "$repo_url"
    elif command -v yum >/dev/null 2>&1; then
        sudo_exec yum install -y "$repo_url"
    else
        print_error "Neither dnf nor yum is available to install RPM repository packages."
        return 1
    fi
}

install_rpm_repo_package_for_channel() {
    local channel=$1
    local repo_url

    repo_url=$(rpm_repo_package_url_for_channel "$channel") || return 1
    if repo_package_installed; then
        remove_rpm_repo_packages
    fi
    install_rpm_repo_package_url "$repo_url"
}

detect_debian_repo_channel() {
    local repo_url

    [[ -s "$DEB_REPO_FILE" ]] || return 1
    repo_url=$(awk 'NF && $1 == "deb" {print $2; exit}' "$DEB_REPO_FILE")
    [[ -n "$repo_url" ]] || return 1

    case "$repo_url" in
        *_dev)
            echo dev
            ;;
        *)
            echo release
            ;;
    esac
}

detect_repo_channel() {
    case "$OS_FAMILY" in
        debian) detect_debian_repo_channel ;;
        *) return 1 ;;
    esac
}

ensure_repo_channel() {
    case "$OS_FAMILY" in
        debian)
            ensure_debian_repo_channel
            ;;
        rpm)
            ensure_rpm_repo_channel
            ;;
        brew)
            if [[ -n "${MANTICORE_REPO_CHANNEL:-}" ]]; then
                print_info "Using Homebrew ${MANTICORE_REPO_CHANNEL} formula: $(brew_package_name)"
            fi
            ;;
    esac
}
ensure_debian_repo_channel() {
    local target="${MANTICORE_REPO_CHANNEL:-}"
    local current="" repo_url deb_path

    current=$(detect_debian_repo_channel || true)

    if [[ -z "$target" ]]; then
        if [[ -n "$current" ]]; then
            if repo_package_installed; then
                print_info "Preserving active Manticore ${current} repository."
            else
                print_warn "${DEB_REPO_FILE} already points to the Manticore ${current} repository, but ${DEB_REPO_PACKAGE_NAME} is not installed. Preserving this manual repository configuration."
            fi
            return 0
        fi
        if repo_package_installed && [[ -z "$current" ]]; then
            print_warn "Manticore repository package is installed, but ${DEB_REPO_FILE} does not identify a known channel. Preserving existing repository state."
            return 0
        fi
        target=release
    fi

    if [[ "$current" == "$target" && repo_package_installed ]]; then
        print_info "Manticore ${target} repository is already active."
        return 0
    fi

    repo_url=$(deb_repo_package_url_for_channel "$target") || return 1
    print_step "Switching Manticore Repository Channel"
    print_info "Activating Manticore ${target} repository."

    deb_path=$(mktemp /tmp/manticore-repo.XXXXXX.deb)
    download_with_available_tool "$repo_url" "$deb_path"
    if repo_package_installed; then
        sudo_exec apt-get remove -y "$DEB_REPO_PACKAGE_NAME"
    fi
    install_debian_repo_package_file "$deb_path" force
    delete_file_if_present "$deb_path"
}

ensure_rpm_repo_channel() {
    local target="${MANTICORE_REPO_CHANNEL:-}"

    if [[ -z "$target" ]]; then
        ensure_repo_package_installed
        print_info "Using RPM repository configuration from the Manticore repository package."
        return 0
    fi

    print_step "Switching Manticore Repository Channel"
    print_info "Activating Manticore ${target} repository."

    if repo_package_installed && detect_rpm_config_manager; then
        set_rpm_repo_channel "$target"
    else
        if repo_package_installed; then
            print_info "RPM repository configuration tool is unavailable; replacing the repository package with the Manticore ${target} repository package."
        else
            print_info "Installing the Manticore ${target} repository package."
        fi
        install_rpm_repo_package_for_channel "$target"
    fi
}

detect_rpm_config_manager() {
    RPM_CONFIG_MANAGER=""

    if command -v dnf >/dev/null 2>&1 && dnf -q config-manager --help >/dev/null 2>&1; then
        RPM_CONFIG_MANAGER="dnf"
        return 0
    fi

    if command -v yum-config-manager >/dev/null 2>&1; then
        RPM_CONFIG_MANAGER="yum"
        return 0
    fi

    return 1
}

ensure_rpm_config_manager() {
    detect_rpm_config_manager && return 0
    print_error "RPM repository configuration tool is unavailable."
    return 1
}

set_rpm_repo_enabled() {
    local repo_id=$1
    local state=$2

    ensure_rpm_config_manager

    if [[ "$RPM_CONFIG_MANAGER" == "dnf" ]]; then
        if [[ "$state" == "enabled" ]]; then
            sudo_exec dnf config-manager --set-enabled "$repo_id"
        else
            sudo_exec dnf config-manager --set-disabled "$repo_id"
        fi
    else
        if [[ "$state" == "enabled" ]]; then
            sudo_exec yum-config-manager --enable "$repo_id"
        else
            sudo_exec yum-config-manager --disable "$repo_id"
        fi
    fi
}

set_rpm_repo_channel() {
    case "$1" in
        dev)
            set_rpm_repo_enabled "$RPM_DEV_REPO_ID" enabled
            set_rpm_repo_enabled "$RPM_RELEASE_REPO_ID" disabled
            ;;
        release)
            set_rpm_repo_enabled "$RPM_RELEASE_REPO_ID" enabled
            set_rpm_repo_enabled "$RPM_DEV_REPO_ID" disabled
            ;;
        *)
            return 1
            ;;
    esac
}
validate_requested_version_argument() {
    local value=${1:-}
    local option_name=${2:-"version"}

    if [[ -n "$value" && "${value:0:1}" == "-" ]]; then
        print_error "$option_name requires a value."
        echo "Run with --help to see supported options." >&2
        exit 2
    fi
}

apt_output_indicates_auth_error() {
    local output_file=$1

    grep -Eiq 'NO_PUBKEY|EXPKEYSIG|BADSIG|GPG error|invalid signature|The following signatures were invalid|signatures couldn.t be verified|public key is not available|repository .* is not signed|InRelease is not signed|Release file is not signed|unauthenticated packages|cannot be authenticated|There is no public key available' "$output_file"
}

apt_update_output_indicates_manticore_repo_issue() {
    local output_file=$1
    local issue_pattern=$2

    awk -v issue_pattern="$issue_pattern" '
        BEGIN {
            manticore = "(repo[.]manticoresearch[.]com|manticoresearch_|manticore-repo)"
        }
        {
            line = $0
            line_lower = tolower(line)
            has_issue = line_lower ~ tolower(issue_pattern)
            has_manticore = line ~ manticore

            if (has_issue && (has_manticore || previous_has_manticore)) {
                found = 1
                exit
            }

            previous_has_manticore = has_manticore
        }
        END { exit found ? 0 : 1 }
    ' "$output_file"
}
apt_update_auth_issue_pattern() {
    printf '%s\n' '(NO_PUBKEY|EXPKEYSIG|BADSIG|GPG error|invalid signature|The following signatures were invalid|signatures couldn.t be verified|public key is not available|repository .* is not signed|InRelease is not signed|Release file is not signed|There is no public key available)'
}

apt_update_repo_problem_pattern() {
    printf '%s\n' '(^Err:|^E:|Failed to fetch|Could not|Temporary failure|Connection failed|Connection timed out|404|does not have a Release file|not signed|NO_PUBKEY|EXPKEYSIG|BADSIG|GPG error|invalid signature|The following signatures were invalid|signatures couldn.t be verified|public key is not available|There is no public key available)'
}

apt_update_output_indicates_manticore_repo_auth_error() {
    apt_update_output_indicates_manticore_repo_issue "$1" "$(apt_update_auth_issue_pattern)"
}

apt_update_output_indicates_manticore_repo_problem() {
    apt_update_output_indicates_manticore_repo_issue "$1" "$(apt_update_repo_problem_pattern)"
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
        if apt_update_output_indicates_manticore_repo_auth_error "$output_file"; then
            print_warn "Apt repository authentication warning detected. Reinstalling the repository bootstrap package and retrying metadata refresh."
            repair_debian_repo_package
            delete_file_if_present "$output_file"
            sudo_exec apt-get update
            return $?
        fi
        delete_file_if_present "$output_file"
        return 0
    fi

    if apt_update_output_indicates_manticore_repo_auth_error "$output_file"; then
        print_warn "Apt repository authentication failed. Reinstalling the repository bootstrap package and retrying metadata refresh."
        repair_debian_repo_package
        delete_file_if_present "$output_file"
        sudo_exec apt-get update
        return $?
    fi
    if ! apt_update_output_indicates_manticore_repo_problem "$output_file"; then
        print_warn "Apt metadata refresh failed for unrelated repositories. Continuing because the Manticore repository did not report an error."
        delete_file_if_present "$output_file"
        return 0
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

    if apt_output_indicates_auth_error "$output_file"; then
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
        local deb_path repo_channel repo_url
        repo_channel="${MANTICORE_REPO_CHANNEL:-release}"
        repo_url=$(deb_repo_package_url_for_channel "$repo_channel") || return 1
        deb_path=$(mktemp /tmp/manticore-repo.XXXXXX.deb)
        download_with_available_tool "$repo_url" "$deb_path"
        install_debian_repo_package_file "$deb_path"
        rm -f "$deb_path"
    elif [[ "$OS_FAMILY" == "rpm" ]]; then
        install_rpm_repo_package_url "$RPM_RELEASE_REPO_PACKAGE_URL"
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
        if [[ -n "${MANTICORE_REPO_CHANNEL:-}" ]]; then
            brew_formula_installed_for_channel "$MANTICORE_REPO_CHANNEL"
        else
            brew_formula_installed_for_channel release || brew_formula_installed_for_channel dev
        fi
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
        brew list --versions "$(brew_package_name)" 2>/dev/null | awk 'NR==1 {print $NF}'
    fi
}

get_latest_available_version() {
    if [[ "$OS_FAMILY" == "debian" ]]; then
        apt-cache madison "$PACKAGE_NAME" 2>/dev/null | awk 'NR==1 {print $3}'
        return 0
    fi

    if [[ "$OS_FAMILY" == "rpm" ]]; then
        if command -v dnf >/dev/null 2>&1; then
            rpm_run dnf --showduplicates list "$PACKAGE_NAME" 2>/dev/null | awk -v pkg="$PACKAGE_NAME" '$1 ~ ("^" pkg "[.]") {print $2}' | tail -n 1
        else
            rpm_run yum --showduplicates list "$PACKAGE_NAME" 2>/dev/null | awk -v pkg="$PACKAGE_NAME" '$1 ~ ("^" pkg "[.]") {print $2}' | tail -n 1
        fi
        return 0
    fi

    if [[ "$OS_FAMILY" == "brew" ]]; then
        brew info --formula "$(brew_package_name)" 2>/dev/null | awk '/^==> / {for (i=1; i<=NF; i++) if ($i == "stable") {print $(i+1); exit}}'
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
            rpm_run dnf --showduplicates list "$PACKAGE_NAME" 2>/dev/null | awk -v pkg="$PACKAGE_NAME" '$1 ~ ("^" pkg "[.]") {print $2}'
        else
            rpm_run yum --showduplicates list "$PACKAGE_NAME" 2>/dev/null | awk -v pkg="$PACKAGE_NAME" '$1 ~ ("^" pkg "[.]") {print $2}'
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

    apt-cache show --no-all-versions "${PACKAGE_NAME}=${version}" 2>/dev/null | awk '
        /^Breaks:/ || /^Provides:/ { field = field " " $0; capture = 1; next }
        capture && /^[[:space:]]/ { field = field " " $0; next }
        capture { capture = 0 }
        END {
            if (field ~ /manticore-(common|server-core|server|tools|dev)/) exit 0
            exit 1
        }
    '
}

debian_dependency_fields_for_package_version() {
    local package=$1
    local version=$2

    apt-cache show --no-all-versions "${package}=${version}" 2>/dev/null | awk '
        /^Depends:/ {
            sub(/^Depends:[ \t]*/, "")
            line = $0
            capture = 1
            next
        }
        capture && /^[ \t]/ {
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

    debian_dependency_fields_for_package_version "$package" "$version" | tr ',' '
' | awk -v version="$version" '
        {
            sub(/^Depends:[ \t]*/, "")
            gsub(/^[ \t]+|[ \t]+$/, "")
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
            if (match($0, /\([^)]+\)/)) {
                constraint = substr($0, RSTART + 1, RLENGTH - 2)
                split(constraint, parts, /[ \t]+/)
                if (parts[1] == "=" && parts[2] == version) print name
            }
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
            gsub(/^[ \t]+|[ \t]+$/, "")
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
        rpm_run dnf -q repoquery --requires "${package}-${version}" 2>/dev/null
    elif command -v repoquery >/dev/null 2>&1; then
        rpm_run repoquery --requires "${package}-${version}" 2>/dev/null
    elif command -v yum >/dev/null 2>&1; then
        rpm_run yum -q repoquery --requires "${package}-${version}" 2>/dev/null
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
            rpm_run dnf --showduplicates list "$package" 2>/dev/null | awk -v pkg="$package" -v version="$version" '$1 ~ ("^" pkg "[.]") && $2 == version {found=1} END {exit found ? 0 : 1}'
        else
            rpm_run yum --showduplicates list "$package" 2>/dev/null | awk -v pkg="$package" -v version="$version" '$1 ~ ("^" pkg "[.]") && $2 == version {found=1} END {exit found ? 0 : 1}'
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
        print_error "The requested Manticore version has an incomplete package set for this OS/repository. Try list-versions or choose another version."
        return 1
    fi
}

version_gt() {
    local left right
    left=$(normalize_version "$1")
    right=$(normalize_version "$2")

    [[ "$(printf '%s\n%s\n' "$right" "$left" | sort -V | tail -n 1)" == "$left" && "$left" != "$right" ]]
}
ensure_service_stopped() {
    local mode=${1:-}

    if service_is_active; then
        print_step "Stopping Manticore Service"
        stop_service
    elif [[ "$mode" != "quiet" ]]; then
        print_info "Manticore service is already stopped."
    fi
}

service_is_active() {
    if [[ "$OS_FAMILY" == "brew" ]]; then
        brew services list 2>/dev/null | awk -v svc="$(brew_service_name)" '$1 == svc && $2 == "started" {found=1} END {exit found ? 0 : 1}'
    elif systemctl_usable; then
        systemctl is-active --quiet "$SERVICE_NAME" >/dev/null 2>&1
    elif command -v pgrep >/dev/null 2>&1; then
        pgrep -x searchd >/dev/null 2>&1
    elif command -v ps >/dev/null 2>&1; then
        ps aux | grep -q '[s]earchd'
    else
        return 1
    fi
}
service_status_observable() {
    [[ "$OS_FAMILY" == "brew" ]] && return 0
    systemctl_usable && return 0
    command -v pgrep >/dev/null 2>&1 && return 0
    command -v ps >/dev/null 2>&1 && return 0
    return 1
}


systemctl_usable() {
    command -v systemctl >/dev/null 2>&1 && systemctl show >/dev/null 2>&1
}

start_service() {
    if [[ "$OS_FAMILY" == "brew" ]]; then
        brew services start "$(brew_package_name)"
    elif systemctl_usable; then
        sudo_exec systemctl start "$SERVICE_NAME"
    else
        sudo_exec mkdir -p /var/run/manticore
        sudo_exec searchd
    fi
}

stop_service() {
    if [[ "$OS_FAMILY" == "brew" ]]; then
        brew services stop "$(brew_package_name)"
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
            rpm_sudo_run dnf makecache
        else
            rpm_sudo_run yum makecache
        fi
    elif [[ "$OS_FAMILY" == "brew" ]]; then
        brew update
    fi
}

if [[ "${BASH_SOURCE[0]}" == "${0}" && "${MANTICORE_STANDALONE:-0}" != "1" ]]; then
    detect_os
    detect_arch
fi
