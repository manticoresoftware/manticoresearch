#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=dist/repo_artifacts.sh
source "$SCRIPT_DIR/repo_artifacts.sh"

usage() {
  cat >&2 <<'EOF'
Usage: resolve_repo_artifact.sh --repo-type TYPE --distr DISTR --arch ARCH --package NAME --sha SHA --extension EXT
EOF
}

repo_type=dev
distr=
arch=
package=manticore-columnar-lib
sha=
extension=

while [[ $# -gt 0 ]]; do
  case "$1" in
    --repo-type) repo_type="${2:?}"; shift 2 ;;
    --distr) distr="${2:?}"; shift 2 ;;
    --arch) arch="${2:?}"; shift 2 ;;
    --package) package="${2:?}"; shift 2 ;;
    --sha) sha="${2:?}"; shift 2 ;;
    --extension) extension="${2:?}"; shift 2 ;;
    --extensions)
      echo "Error: use --extension with a single extension" >&2
      exit 2
      ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Error: unknown argument $1" >&2; usage; exit 2 ;;
  esac
done

if [[ -z "$distr" || -z "$arch" || -z "$package" || -z "$sha" || -z "$extension" ]]; then
  usage
  exit 2
fi

[[ "$extension" == .* ]] || extension=".$extension"
ra_resolve_repo_artifact "$repo_type" "$distr" "$arch" "$package" "$sha" "$extension"
