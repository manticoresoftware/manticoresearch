#!/bin/bash
# Download and extract dependency packages from repo.manticoresearch.com into OUT_DIR.
set -euo pipefail

REPO_ROOT="${REPO_ROOT:-$(realpath "$(dirname "$0")/..")}"
OUT_DIR="${1:?Usage: $0 <output_dir>}"
DEPS_FILE="${REPO_ROOT}/deps.txt"
REPO_TYPE="${BUNDLE_REPO_TYPE:-dev}"
DISTR="${DISTR:?DISTR must be set (e.g. jammy, rhel9, macos, windows)}"
arch="${arch:?arch must be set (e.g. x86_64, aarch64)}"
MCL_PACKAGE_DIR="${MCL_PACKAGE_DIR:-}"
MCL_PACKAGE_SHA="${MCL_PACKAGE_SHA:-}"
MCL_REPO_TYPE="${MCL_REPO_TYPE:-$REPO_TYPE}"

[[ -f "$DEPS_FILE" ]] || { echo "Error: deps.txt not found at $DEPS_FILE" >&2; exit 1; }
mkdir -p "$OUT_DIR"

should_skip() {
  local pkg="$1"
  [[ "$DISTR" == "windows" && "$pkg" == "executor" ]] && return 0
  [[ "$DISTR" == "windows" && "$pkg" == "galera" ]] && return 0
  [[ "$DISTR" == "macos" && "$pkg" == "tzdata" ]] && return 0
  return 1
}

real_name() {
  case "$1" in
    backup) echo manticore-backup ;;
    buddy) echo manticore-buddy ;;
    load) echo manticore-load ;;
    mcl) echo manticore-columnar-lib ;;
    tzdata) echo manticore-tzdata ;;
    executor) echo manticore-executor ;;
    galera) echo manticore-galera ;;
    *) echo "$1" ;;
  esac
}

package_arch_class() {
  case "$1" in
    manticore-backup|manticore-buddy|manticore-load|manticore-tzdata) echo all ;;
    *) echo native ;;
  esac
}

is_arm_arch() {
  [[ "$arch" == "aarch64" || "$arch" == "arm64" ]]
}

is_x64_arch() {
  [[ "$arch" == "x86_64" || "$arch" == "amd64" || "$arch" == "x64" ]]
}

get_repo_ctx() {
  local deb_arch="$arch"
  is_x64_arch && deb_arch=amd64
  is_arm_arch && deb_arch=arm64
  if [[ "$DISTR" == "windows" ]]; then
    echo "https://repo.manticoresearch.com/repository/manticoresearch_windows/${REPO_TYPE}/x64"; echo zip; echo ""
  elif [[ "$DISTR" == "macos" ]]; then
    echo "https://repo.manticoresearch.com/repository/manticoresearch_macos/${REPO_TYPE}"; echo tar.gz; echo ""
  elif [[ "$DISTR" =~ ^rhel[0-9]+$ ]]; then
    local rhel="${DISTR#rhel}"
    echo "https://repo.manticoresearch.com/repository/manticoresearch/${REPO_TYPE}/centos/${rhel}/${arch}"; echo rpm; echo ""
  else
    local root="https://repo.manticoresearch.com/repository/manticoresearch_${DISTR}_${REPO_TYPE}"
    echo "${root}/dists/${DISTR}/main/binary-${deb_arch}"; echo deb; echo "$root"
  fi
}

repo_ctx=()
while IFS= read -r line; do
  repo_ctx+=("$line")
done < <(get_repo_ctx)
base_url="${repo_ctx[0]:-}"
pkg_fmt="${repo_ctx[1]:-}"
deb_repo_root="${repo_ctx[2]:-}"

need_cmd() { command -v "$1" >/dev/null 2>&1 || { echo "Error: required command '$1' is missing" >&2; exit 127; }; }
case "$pkg_fmt" in
  deb) need_cmd dpkg-deb ;;
  rpm) need_cmd rpm2cpio; need_cmd cpio; need_cmd rpm ;;
  tar.gz) need_cmd tar ;;
  zip) need_cmd unzip ;;
esac
[[ "$pkg_fmt" != "deb" ]] || need_cmd gzip

# Try wget then curl, up to 3 rounds.
download_retry() {
  local url="$1" out="$2" i
  for i in 1 2 3; do
    wget -q -T 60 -O "$out" "$url" 2>/dev/null && [[ -s "$out" ]] && return 0
    curl -sfL --connect-timeout 30 --max-time 120 -o "$out" "$url" 2>/dev/null && [[ -s "$out" ]] && return 0
    rm -f "$out"; [[ $i -lt 3 ]] && sleep 2
  done
  rm -f "$out"; return 1
}

download_text() {
  local url="$1" t
  t=$(mktemp)
  download_retry "$url" "$t" || { rm -f "$t"; return 1; }
  cat "$t"; rm -f "$t"
}

parse_dep_spec() {
  local rest="$1"
  DEP_VERSION=""; DEP_DATE=""; DEP_COMMIT=""; DEP_SUFFIX=""; DEP_VERSION_STRING=""
  if [[ "$rest" =~ ^([^[:space:]]+)\+([0-9]+)-([a-f0-9]+)(-([a-zA-Z0-9]+))?$ ]]; then
    DEP_VERSION="${BASH_REMATCH[1]}"; DEP_DATE="${BASH_REMATCH[2]}"; DEP_COMMIT="${BASH_REMATCH[3]}"; DEP_SUFFIX="${BASH_REMATCH[5]:-}"
    DEP_VERSION_STRING="${DEP_VERSION}+${DEP_DATE}-${DEP_COMMIT}${DEP_SUFFIX:+-$DEP_SUFFIX}"; return 0
  fi
  if [[ "$rest" =~ ^([^[:space:]]+)[[:space:]]+([0-9]+)[[:space:]]+([a-f0-9]+)$ ]]; then
    DEP_VERSION="${BASH_REMATCH[1]}"; DEP_DATE="${BASH_REMATCH[2]}"; DEP_COMMIT="${BASH_REMATCH[3]}"
    DEP_VERSION_STRING="${DEP_VERSION}-${DEP_DATE}-${DEP_COMMIT}"; return 0
  fi
  if [[ "$rest" =~ ^([^[:space:]]+)$ ]]; then
    DEP_VERSION="${BASH_REMATCH[1]}"; DEP_VERSION_STRING="$DEP_VERSION"; return 0
  fi
  return 1
}

trim_spaces() {
  local s="$1"
  s="${s#"${s%%[![:space:]]*}"}"
  s="${s%"${s##*[![:space:]]}"}"
  printf '%s' "$s"
}

is_internal_dep() {
  local dep="$1"
  [[ "$dep" =~ ^(manticore-backup|manticore-buddy|manticore-load|manticore-executor|manticore-galera|manticore-columnar-lib|manticore-tzdata)([[:space:]]|:|\(|$) ]]
}

declare -a META_ENTRIES=()
declare -a BUNDLED_PACKAGE_NAMES=()
declare -a BUNDLED_PACKAGE_PROVIDES=()

meta_has_entry() {
  local needle="$1"
  local entry
  for entry in "${META_ENTRIES[@]}"; do
    [[ "$entry" == "$needle" ]] && return 0
  done
  return 1
}

add_meta_entry() {
  local raw="$1"
  local dep
  dep="$(trim_spaces "$raw")"
  [[ -n "$dep" ]] || return 0
  if [[ "$pkg_fmt" == "rpm" && "$dep" == rpmlib\(* ]]; then
    return 0
  fi
  if is_internal_dep "$dep"; then
    return 0
  fi
  if ! meta_has_entry "$dep"; then
    META_ENTRIES+=("$dep")
  fi
}

add_bundled_package_name() {
  local pkg_name="$1"
  [[ -n "$pkg_name" ]] || return 0
  local entry
  for entry in "${BUNDLED_PACKAGE_NAMES[@]}"; do
    [[ "$entry" == "$pkg_name" ]] && return 0
  done
  BUNDLED_PACKAGE_NAMES+=("$pkg_name")
}

add_bundled_package_provide() {
  local provide="$1"
  [[ -n "$provide" ]] || return 0
  local entry
  for entry in "${BUNDLED_PACKAGE_PROVIDES[@]}"; do
    [[ "$entry" == "$provide" ]] && return 0
  done
  BUNDLED_PACKAGE_PROVIDES+=("$provide")
}

collect_external_deps() {
  local pkg_path="$1"
  if [[ "$pkg_fmt" == "deb" ]]; then
    local field raw deb_pkg_name deb_pkg_version
    deb_pkg_name=$(dpkg-deb -f "$pkg_path" Package 2>/dev/null || true)
    deb_pkg_version=$(dpkg-deb -f "$pkg_path" Version 2>/dev/null || true)
    add_bundled_package_name "$deb_pkg_name"
    [[ -n "$deb_pkg_name" && -n "$deb_pkg_version" ]] && add_bundled_package_provide "${deb_pkg_name} (= ${deb_pkg_version})"
    for field in Depends Pre-Depends; do
      raw=$(dpkg-deb -f "$pkg_path" "$field" 2>/dev/null || true)
      [[ -n "$raw" ]] || continue
      raw="${raw//$'\n'/ }"
      local item
      IFS=',' read -ra items <<< "$raw"
      for item in "${items[@]}"; do
        add_meta_entry "$item"
      done
    done
    return 0
  fi
  if [[ "$pkg_fmt" == "rpm" ]]; then
    local rpm_pkg_name rpm_pkg_version
    rpm_pkg_name=$(rpm -qp --qf '%{NAME}\n' "$pkg_path" 2>/dev/null || true)
    rpm_pkg_version=$(rpm -qp --qf '%{VERSION}-%{RELEASE}\n' "$pkg_path" 2>/dev/null || true)
    add_bundled_package_name "$rpm_pkg_name"
    [[ -n "$rpm_pkg_name" && -n "$rpm_pkg_version" ]] && add_bundled_package_provide "${rpm_pkg_name} = ${rpm_pkg_version}"
    while IFS= read -r req; do
      add_meta_entry "$req"
    done < <(rpm -qp --requires "$pkg_path" 2>/dev/null || true)
  fi
}

extract_one() {
  local p="$1"
  local pkg_name="${2:-}"
  case "$pkg_fmt" in
    deb) dpkg-deb -x "$p" "$OUT_DIR" ;;
    rpm) (cd "$OUT_DIR" && rpm2cpio "$p" | cpio -idmv) ;;
    tar.gz)
      if [[ "$DISTR" != "macos" ]]; then
        tar -xzf "$p" -C "$OUT_DIR"
      else
        local tmp
        tmp=$(mktemp -d)
        tar -xzf "$p" -C "$tmp"

        # Strip a single package wrapper directory when archive is wrapped.
        local entries=()
        while IFS= read -r line; do
          entries+=("$line")
        done < <(find "$tmp" -mindepth 1 -maxdepth 1 -print)
        if [[ ${#entries[@]} -eq 1 && -d "${entries[0]}" ]]; then
          local wrapped="${entries[0]}"
          local tmp2
          tmp2=$(mktemp -d)
          (cd "$wrapped" && tar -cf - .) | (cd "$tmp2" && tar -xf -)
          rm -rf "$tmp"
          tmp="$tmp2"
        fi

        # Normalize known homebrew/install prefixes to bundle root.
        if [[ -d "$tmp/usr/local" ]]; then
          local usr_local_items=()
          while IFS= read -r line; do
            usr_local_items+=("$line")
          done < <(find "$tmp/usr/local" -mindepth 1 -maxdepth 1 -print)
          [[ ${#usr_local_items[@]} -gt 0 ]] && mv "${usr_local_items[@]}" "$tmp"/
          rm -rf "$tmp/usr"
        fi
        if [[ -d "$tmp/opt/homebrew" ]]; then
          local homebrew_items=()
          while IFS= read -r line; do
            homebrew_items+=("$line")
          done < <(find "$tmp/opt/homebrew" -mindepth 1 -maxdepth 1 -print)
          [[ ${#homebrew_items[@]} -gt 0 ]] && mv "${homebrew_items[@]}" "$tmp"/
          rm -rf "$tmp/opt"
        fi

        # Normalize module payloads to share/manticore/modules on macOS.
        if [[ -d "$tmp/share/modules" ]]; then
          mkdir -p "$tmp/share/manticore/modules"
          find "$tmp/share/modules" -mindepth 1 -maxdepth 1 -exec mv {} "$tmp/share/manticore/modules/" \;
          rm -rf "$tmp/share/modules"
        fi

        case "$pkg_name" in
          manticore-buddy)
            mkdir -p "$tmp/share/manticore/modules/manticore-buddy/bin"
            [[ -f "$tmp/bin/manticore-buddy" ]] && mv "$tmp/bin/manticore-buddy" "$tmp/share/manticore/modules/manticore-buddy/bin/"
            rmdir "$tmp/bin" 2>/dev/null || true
            ;;
          manticore-executor)
            mkdir -p "$tmp/bin" "$tmp/share/doc/manticore/manticore-executor"
            [[ -f "$tmp/manticore-executor" ]] && mv "$tmp/manticore-executor" "$tmp/bin/"
            for f in APP_VERSION LICENSE README.md THIRD-PARTY-LICENSES; do
              [[ -e "$tmp/$f" ]] && mv "$tmp/$f" "$tmp/share/doc/manticore/manticore-executor/"
            done
            ;;
          manticore-load)
            mkdir -p "$tmp/bin" "$tmp/share/manticore/modules/manticore-load" "$tmp/share/doc/manticore/manticore-load"
            [[ -f "$tmp/manticore-load" ]] && mv "$tmp/manticore-load" "$tmp/bin/"
            [[ -f "$tmp/APP_VERSION" ]] && mv "$tmp/APP_VERSION" "$tmp/share/manticore/modules/manticore-load/"
            for f in LICENSE README.md THIRD-PARTY-LICENSES; do
              [[ -e "$tmp/$f" ]] && mv "$tmp/$f" "$tmp/share/doc/manticore/manticore-load/"
            done
            if [[ -d "$tmp/src" ]]; then
              find "$tmp/src" -mindepth 1 -maxdepth 1 -exec mv {} "$tmp/share/manticore/modules/manticore-load/" \;
              rm -rf "$tmp/src"
            fi
            ;;
        esac

        # Merge normalized tree into OUT_DIR.
        (cd "$tmp" && tar -cf - .) | (cd "$OUT_DIR" && tar -xf -)
        rm -rf "$tmp"
      fi
      ;;
    zip) unzip -o -q "$p" -d "$OUT_DIR" ;;
  esac
}

DEB_PACKAGES_CACHE_ARCH=""
DEB_PACKAGES_CACHE_TEXT=""

deb_repo_arch() { is_arm_arch && echo arm64 || echo amd64; }

deb_pkg_arch() {
  local arch_class="$1"
  [[ "$arch_class" == all ]] && echo all || deb_repo_arch
}

get_deb_packages() {
  local deb_arch="$1" text=""
  if [[ "$DEB_PACKAGES_CACHE_ARCH" == "$deb_arch" && -n "$DEB_PACKAGES_CACHE_TEXT" ]]; then
    printf '%s' "$DEB_PACKAGES_CACHE_TEXT"; return 0
  fi
  local gz="${deb_repo_root}/dists/${DISTR}/main/binary-${deb_arch}/Packages.gz"
  local plain="${deb_repo_root}/dists/${DISTR}/main/binary-${deb_arch}/Packages"
  local t
  t=$(mktemp)
  if download_retry "$gz" "$t"; then text=$(gzip -dc "$t" 2>/dev/null || true); fi
  rm -f "$t"
  [[ -n "$text" ]] || text=$(download_text "$plain" || true)
  [[ -n "$text" ]] || { echo "Error: could not fetch Packages(.gz) for binary-${deb_arch}" >&2; return 1; }
  DEB_PACKAGES_CACHE_ARCH="$deb_arch"; DEB_PACKAGES_CACHE_TEXT="$text"; printf '%s' "$text"
}

resolve_deb_url() {
  local real_pkg="$1" repo_arch="$2" pkg_arch="$3" pkgs
  pkgs=$(get_deb_packages "$repo_arch") || return 1
  local fn
  fn=$(awk -v p="$real_pkg" -v a="$pkg_arch" -v v="$DEP_VERSION" -v d="$DEP_DATE" -v c="$DEP_COMMIT" -v s="$DEP_SUFFIX" -v e="$DEP_VERSION_STRING" '
BEGIN{RS="";FS="\n";best=-1}
{
  P=V=A=F=""; for(i=1;i<=NF;i++){if($i~/^Package: /)P=substr($i,10); else if($i~/^Version: /)V=substr($i,10); else if($i~/^Architecture: /)A=substr($i,15); else if($i~/^Filename: /)F=substr($i,11)}
  if(P!=p||A!=a||F==""||index(V,v)==0)next
  if(d!=""&&index(V,d)==0)next; if(c!=""&&index(V,c)==0)next; if(s!=""&&index(V,s)==0)next
  S=(V==e?100:0)+(d!=""?10:0)+(c!=""?10:0)+(s!=""?5:0)
  if(S>best){best=S;bestF=F}
}
END{if(bestF!="")print bestF}
' <<< "$pkgs")
  [[ -n "$fn" ]] || { echo "Error: no matching deb artifact for $real_pkg ($DEP_VERSION_STRING) in Packages" >&2; return 1; }
  echo "${deb_repo_root}/${fn}"
}

LISTING_HTML=""
get_listing_html() {
  [[ -n "$LISTING_HTML" ]] || LISTING_HTML=$(download_text "${base_url}/" || true)
  [[ -n "$LISTING_HTML" ]] || { echo "Error: could not fetch listing ${base_url}/" >&2; return 1; }
  printf '%s' "$LISTING_HTML"
}

list_arch_tokens() {
  local arch_class="$1"
  [[ "$arch_class" == all ]] && { echo ""; return; }
  case "$pkg_fmt" in
    rpm) is_arm_arch && echo "aarch64 arm64" || echo "x86_64 amd64" ;;
    tar.gz) is_arm_arch && echo "arm64 aarch64" || echo "x86_64 amd64" ;;
    zip) is_arm_arch && echo "arm64 aarch64" || echo "x64 x86_64 amd64" ;;
    *) echo "" ;;
  esac
}

resolve_listing_url() {
  local real_pkg="$1" arch_class="$2" html tokens best="" best_score=-1 ties=0
  local required_zip_suffix=""
  if [[ "$pkg_fmt" == zip ]]; then
    case "$real_pkg" in
      manticore-columnar-lib|manticore-executor) required_zip_suffix="-libs.zip" ;;
    esac
  fi
  html=$(get_listing_html) || return 1
  tokens=$(list_arch_tokens "$arch_class")
  while IFS= read -r href; do
    href="${href%%\?*}"; [[ -n "$href" && "$href" != "../" ]] || continue
    local name dec score=0 tok
    name="${href##*/}"
    dec="${name//%2B/+}"
    dec="${dec//%2b/+}"
    [[ "$pkg_fmt" == tar.gz ]] && [[ "$dec" == *.tar.gz ]] || [[ "$pkg_fmt" != tar.gz && "$dec" == *."$pkg_fmt" ]] || continue
    [[ "$dec" == "${real_pkg}-${DEP_VERSION}"* || "$dec" == "${real_pkg}_${DEP_VERSION}"* ]] || continue
    [[ -z "$DEP_DATE" || "$dec" == *"$DEP_DATE"* ]] || continue
    [[ -z "$DEP_COMMIT" || "$dec" == *"$DEP_COMMIT"* ]] || continue
    [[ -z "$DEP_SUFFIX" || "$dec" == *"$DEP_SUFFIX"* ]] || continue
    [[ "$pkg_fmt" != zip || "$dec" != *dbgsymbols* ]] || continue
    [[ -z "$required_zip_suffix" || "$dec" == *"$required_zip_suffix" ]] || continue
    [[ "$dec" == *"${real_pkg}-${DEP_VERSION_STRING}"* || "$dec" == *"${real_pkg}_${DEP_VERSION_STRING}"* ]] && score=$((score+10))
    [[ -n "$DEP_DATE" ]] && score=$((score+5)); [[ -n "$DEP_COMMIT" ]] && score=$((score+5)); [[ -n "$DEP_SUFFIX" ]] && score=$((score+3))
    for tok in $tokens; do [[ "$dec" == *"$tok"* ]] && { score=$((score+4)); break; }; done
    if (( score > best_score )); then best_score=$score; best="$href"; ties=1
    elif (( score == best_score )); then ties=$((ties+1)); fi
  done < <(printf '%s\n' "$html" | sed -nE 's/.*href="([^"]+)".*/\1/p')
  [[ -n "$best" ]] || { echo "Error: no matching ${pkg_fmt} artifact for $real_pkg ($DEP_VERSION_STRING) in ${base_url}/" >&2; return 1; }
  (( ties == 1 )) || { echo "Error: ambiguous ${pkg_fmt} artifact for $real_pkg ($DEP_VERSION_STRING) in ${base_url}/" >&2; return 1; }
  [[ "$best" =~ ^https?:// ]] && echo "$best" || echo "${base_url}/${best}"
}


process_package_file() {
  local pkg_path="$1" pkg_name="$2"
  collect_external_deps "$pkg_path"
  extract_one "$pkg_path" "$pkg_name"
}

find_local_mcl_package() {
  [[ -n "$MCL_PACKAGE_DIR" && -d "$MCL_PACKAGE_DIR" ]] || return 1
  case "$pkg_fmt" in
    deb) find "$MCL_PACKAGE_DIR" -type f -name 'manticore-columnar-lib_*.'"$pkg_fmt" 2>/dev/null | grep -v dbgsym | head -n 1 ;;
    rpm) find "$MCL_PACKAGE_DIR" -type f -name 'manticore-columnar-lib-*.'"$pkg_fmt" 2>/dev/null | grep -v debuginfo | head -n 1 ;;
    tar.gz) find "$MCL_PACKAGE_DIR" -type f -name 'manticore-columnar-lib-*.'"$pkg_fmt" 2>/dev/null | grep -v dbgsymbols | head -n 1 ;;
    zip) find "$MCL_PACKAGE_DIR" -type f -name 'manticore-columnar-lib-*.zip' 2>/dev/null | grep '\-libs\.zip$' | head -n 1 ;;
  esac
}

install_mcl_package() {
  local real_pkg="manticore-columnar-lib"
  local local_pkg artifact_url artifact_name out_path
  local_pkg=$(find_local_mcl_package || true)
  if [[ -n "$local_pkg" ]]; then
    echo "Using locally built MCL package $local_pkg"
    process_package_file "$local_pkg" "$real_pkg"
    return 0
  fi

  [[ -n "$MCL_PACKAGE_SHA" ]] || { echo "Error: MCL package is required, but neither MCL_PACKAGE_DIR nor MCL_PACKAGE_SHA was provided" >&2; return 1; }
  echo "Resolving MCL package by sha ${MCL_PACKAGE_SHA} from repo.manticoresearch.com ..."
  artifact_url=$(python3 "${REPO_ROOT}/dist/resolve_repo_package.py" --repo-type "$MCL_REPO_TYPE" --distr "$DISTR" --arch "$arch" --sha "$MCL_PACKAGE_SHA" --extensions ".${pkg_fmt}") || return 1
  artifact_url="${artifact_url//+/%2B}"
  artifact_name="${artifact_url##*/}"
  out_path="$OUT_DIR/$artifact_name"
  echo "Downloading $artifact_name"
  download_retry "$artifact_url" "$out_path" || { echo "Error: download failed for $artifact_url" >&2; return 1; }
  process_package_file "$out_path" "$real_pkg"
  rm -f "$out_path"
}

while IFS= read -r line || [[ -n "$line" ]]; do
  [[ -z "$line" || "$line" =~ ^[[:space:]#] ]] && continue
  [[ "$line" == "---" ]] && break
  package="${line%%[[:space:]]*}"; rest="${line#*[[:space:]]}"
  [[ "$package" =~ ^[a-z]+$ ]] || continue
  should_skip "$package" && { echo "Skipping $package for $DISTR (per skip list)"; continue; }

  real_pkg=$(real_name "$package")
  parse_dep_spec "$rest" || { echo "Skipping $package: could not parse version from '$rest'" >&2; continue; }

  arch_class=$(package_arch_class "$real_pkg")
  echo "Resolving $real_pkg $DEP_VERSION_STRING from repository index ..."
  if [[ "$pkg_fmt" == deb ]]; then
    artifact_url=$(resolve_deb_url "$real_pkg" "$(deb_repo_arch)" "$(deb_pkg_arch "$arch_class")") || exit 1
  else
    artifact_url=$(resolve_listing_url "$real_pkg" "$arch_class") || exit 1
  fi

  artifact_url="${artifact_url//+/%2B}"
  artifact_name="${artifact_url##*/}"
  out_path="$OUT_DIR/$artifact_name"
  echo "Downloading $artifact_name"
  download_retry "$artifact_url" "$out_path" || { echo "Error: download failed for $artifact_url" >&2; exit 1; }
  process_package_file "$out_path" "$real_pkg"
  rm -f "$out_path"
done < "$DEPS_FILE"

install_mcl_package

meta_file=""
case "$pkg_fmt" in
  deb) meta_file="$OUT_DIR/.bundle_depends.deb" ;;
  rpm) meta_file="$OUT_DIR/.bundle_requires.rpm" ;;
esac
if [[ -n "$meta_file" ]]; then
  if [[ ${#META_ENTRIES[@]} -gt 0 ]]; then
    printf '%s\n' "${META_ENTRIES[@]}" | sort -u > "$meta_file"
    echo "Saved external dependency metadata to $meta_file"
  else
    rm -f "$meta_file"
  fi
fi

owned_pkgs_file=""
case "$pkg_fmt" in
  deb) owned_pkgs_file="$OUT_DIR/.bundle_owned_packages.deb" ;;
  rpm) owned_pkgs_file="$OUT_DIR/.bundle_owned_packages.rpm" ;;
esac
if [[ -n "$owned_pkgs_file" ]]; then
  if [[ ${#BUNDLED_PACKAGE_NAMES[@]} -gt 0 ]]; then
    printf '%s\n' "${BUNDLED_PACKAGE_NAMES[@]}" | sort -u > "$owned_pkgs_file"
    echo "Saved bundled package ownership metadata to $owned_pkgs_file"
  else
    rm -f "$owned_pkgs_file"
  fi
fi

provides_file=""
case "$pkg_fmt" in
  deb) provides_file="$OUT_DIR/.bundle_provides.deb" ;;
  rpm) provides_file="$OUT_DIR/.bundle_provides.rpm" ;;
esac
if [[ -n "$provides_file" ]]; then
  if [[ ${#BUNDLED_PACKAGE_PROVIDES[@]} -gt 0 ]]; then
    printf '%s\n' "${BUNDLED_PACKAGE_PROVIDES[@]}" | sort -u > "$provides_file"
    echo "Saved bundled package provides metadata to $provides_file"
  else
    rm -f "$provides_file"
  fi
fi

echo "Bundle deps extracted to $OUT_DIR"
