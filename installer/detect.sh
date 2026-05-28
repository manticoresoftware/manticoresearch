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
    local downloaded_package downloaded_version installed_version

    downloaded_package=$(dpkg-deb -f "$deb_path" Package 2>/dev/null || true)
    downloaded_version=$(dpkg-deb -f "$deb_path" Version 2>/dev/null || true)

    if [[ "$downloaded_package" != "$DEB_REPO_PACKAGE_NAME" || -z "$downloaded_version" ]]; then
        print_error "Downloaded repository package is not a valid ${DEB_REPO_PACKAGE_NAME} package."
        return 1
    fi

    installed_version=$(dpkg-query -W -f='${Version}\n' "$DEB_REPO_PACKAGE_NAME" 2>/dev/null || true)
    if [[ -n "$installed_version" && "$installed_version" == "$downloaded_version" ]]; then
        print_info "Repository bootstrap package ${DEB_REPO_PACKAGE_NAME} ${installed_version} is already installed."
        return 0
    fi

    sudo_exec dpkg -i "$deb_path"
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
            sub(/[[:space:](].*/, "", name)
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
            sub(/[[:space:](].*/, "", name)
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
        sudo_exec apt-get update
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

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    detect_os
    detect_arch
fi
