#!/bin/bash

set -euo pipefail

SCRIPT_DIR=$(dirname -- "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(cd "$SCRIPT_DIR" && pwd)
OUTPUT_PATH=${1:-"$SCRIPT_DIR/bootstrap-standalone.sh"}
TMP_PATH=$(mktemp "${OUTPUT_PATH}.XXXXXX")

cleanup() {
    rm -f "$TMP_PATH"
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
} > "$TMP_PATH"

chmod +x "$TMP_PATH"
bash -n "$TMP_PATH"
mv "$TMP_PATH" "$OUTPUT_PATH"
trap - EXIT

echo "Built $OUTPUT_PATH"
