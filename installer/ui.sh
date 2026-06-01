#!/bin/bash

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
