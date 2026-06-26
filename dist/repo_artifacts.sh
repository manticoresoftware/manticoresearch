#!/usr/bin/env bash
# Shared helpers for resolving artifacts from repo.manticoresearch.com.
# This file is intended to be sourced by bash scripts.

ra_is_arm_arch() {
  local arch="$1"
  [[ "$arch" == "aarch64" || "$arch" == "arm64" ]]
}

ra_is_x64_arch() {
  local arch="$1"
  [[ "$arch" == "x86_64" || "$arch" == "amd64" || "$arch" == "x64" ]]
}

ra_deb_arch() {
  local arch="$1"
  if ra_is_arm_arch "$arch"; then
    echo arm64
  else
    echo amd64
  fi
}

ra_repo_context() {
  local repo_type="$1" distr="$2" arch="$3"
  local deb_arch="$arch"
  ra_is_x64_arch "$arch" && deb_arch=amd64
  ra_is_arm_arch "$arch" && deb_arch=arm64

  if [[ "$distr" == "windows" ]]; then
    echo "https://repo.manticoresearch.com/repository/manticoresearch_windows/${repo_type}/x64"
    echo zip
    echo ""
  elif [[ "$distr" == "macos" ]]; then
    echo "https://repo.manticoresearch.com/repository/manticoresearch_macos/${repo_type}"
    echo tar.gz
    echo ""
  elif [[ "$distr" =~ ^rhel[0-9]+$ ]]; then
    local rhel="${distr#rhel}"
    echo "https://repo.manticoresearch.com/repository/manticoresearch/${repo_type}/centos/${rhel}/${arch}"
    echo rpm
    echo ""
  else
    local root="https://repo.manticoresearch.com/repository/manticoresearch_${distr}_${repo_type}"
    echo "${root}/dists/${distr}/main/binary-${deb_arch}"
    echo deb
    echo "$root"
  fi
}

ra_download_retry() {
  local url="$1" out="$2" i
  for i in 1 2 3; do
    if command -v wget >/dev/null 2>&1 && wget -q -T 60 -O "$out" "$url" 2>/dev/null && [[ -s "$out" ]]; then
      return 0
    fi
    if command -v curl >/dev/null 2>&1 && curl -sfL --connect-timeout 30 --max-time 120 -o "$out" "$url" 2>/dev/null && [[ -s "$out" ]]; then
      return 0
    fi
    rm -f "$out"
    [[ $i -lt 3 ]] && sleep 2
  done
  rm -f "$out"
  return 1
}

ra_download_text() {
  local url="$1" t
  t=$(mktemp)
  ra_download_retry "$url" "$t" || { rm -f "$t"; return 1; }
  cat "$t"
  rm -f "$t"
}

ra_url_escape() {
  local url="$1"
  echo "${url//+/%2B}"
}

ra_list_arch_tokens() {
  local pkg_fmt="$1" arch="$2"
  case "$pkg_fmt" in
    rpm) ra_is_arm_arch "$arch" && echo "aarch64 arm64" || echo "x86_64 amd64" ;;
    tar.gz) ra_is_arm_arch "$arch" && echo "arm64 aarch64" || echo "x86_64 amd64" ;;
    zip) ra_is_arm_arch "$arch" && echo "arm64 aarch64" || echo "x64 x86_64 amd64" ;;
    *) echo "" ;;
  esac
}

ra_extension_matches() {
  local name="$1" extension="$2"
  [[ "$name" == *"$extension" ]]
}

ra_resolve_deb_artifact_by_sha() {
  local repo_type="$1" distr="$2" arch="$3" package="$4" sha="$5" extension="$6" deb_repo_root="$7"
  local deb_arch packages plain gz t matches
  deb_arch=$(ra_deb_arch "$arch")
  gz="${deb_repo_root}/dists/${distr}/main/binary-${deb_arch}/Packages.gz"
  plain="${deb_repo_root}/dists/${distr}/main/binary-${deb_arch}/Packages"
  t=$(mktemp)
  packages=""
  if ra_download_retry "$gz" "$t"; then
    packages=$(gzip -dc "$t" 2>/dev/null || true)
  fi
  rm -f "$t"
  [[ -n "$packages" ]] || packages=$(ra_download_text "$plain" || true)
  [[ -n "$packages" ]] || { echo "Error: could not fetch Packages(.gz) for ${distr} binary-${deb_arch}" >&2; return 1; }

  matches=$(awk -v p="$package" -v a="$deb_arch" -v sha="$(echo "$sha" | tr '[:upper:]' '[:lower:]')" -v ext="$extension" '
BEGIN{RS="";FS="\n"}
{
  P=V=A=F="";
  for(i=1;i<=NF;i++){
    if($i~/^Package: /)P=substr($i,10);
    else if($i~/^Version: /)V=substr($i,10);
    else if($i~/^Architecture: /)A=substr($i,15);
    else if($i~/^Filename: /)F=substr($i,11);
  }
  L=tolower(V " " F);
  if(P==p && A==a && F!="" && index(L, sha)>0 && substr(F, length(F)-length(ext)+1)==ext) print F;
}
' <<< "$packages")

  local count
  count=$(printf '%s\n' "$matches" | sed '/^$/d' | wc -l)
  if [[ "$count" -eq 0 ]]; then
    echo "Error: no ${extension} artifact found for ${package} sha=${sha} in ${deb_repo_root}" >&2
    return 1
  fi
  if [[ "$count" -ne 1 ]]; then
    echo "Error: ambiguous ${extension} artifacts for ${package} sha=${sha} in ${deb_repo_root}" >&2
    printf '%s\n' "$matches" >&2
    return 1
  fi
  ra_url_escape "${deb_repo_root}/$(printf '%s\n' "$matches" | sed '/^$/d')"
}

ra_resolve_listing_artifact_by_sha() {
  local base_url="$1" pkg_fmt="$2" arch="$3" package="$4" sha="$5" extension="$6"
  local html tokens matches count
  html=$(ra_download_text "${base_url}/") || { echo "Error: could not fetch listing ${base_url}/" >&2; return 1; }
  tokens=$(ra_list_arch_tokens "$pkg_fmt" "$arch")

  matches=$(
    while IFS= read -r href; do
      href="${href%%\?*}"
      [[ -n "$href" && "$href" != "../" ]] || continue
      local name dec lname tok token_ok
      name="${href##*/}"
      dec="${name//%2B/+}"
      dec="${dec//%2b/+}"
      lname=$(echo "$dec" | tr '[:upper:]' '[:lower:]')
      [[ "$lname" == *"$(echo "$package" | tr '[:upper:]' '[:lower:]')"* ]] || continue
      [[ "$lname" == *"$(echo "$sha" | tr '[:upper:]' '[:lower:]')"* ]] || continue
      ra_extension_matches "$lname" "$extension" || continue
      [[ "$lname" != *dbgsymbols* && "$lname" != *debuginfo* ]] || continue
      if [[ "$pkg_fmt" == zip && ( "$package" == "manticore-columnar-lib" || "$package" == "manticore-executor" ) ]]; then
        [[ "$lname" == *-libs.zip ]] || continue
      fi
      token_ok=false
      if [[ -z "$tokens" ]]; then
        token_ok=true
      else
        for tok in $tokens; do
          [[ "$lname" == *"$tok"* ]] && { token_ok=true; break; }
        done
      fi
      [[ "$token_ok" == true ]] || continue
      [[ "$href" =~ ^https?:// ]] && echo "$href" || echo "${base_url}/${href}"
    done < <(printf '%s\n' "$html" | sed -nE 's/.*href="([^"]+)".*/\1/p')
  )

  count=$(printf '%s\n' "$matches" | sed '/^$/d' | wc -l)
  if [[ "$count" -eq 0 ]]; then
    echo "Error: no ${extension} artifact found for ${package} sha=${sha} in ${base_url}/" >&2
    return 1
  fi
  if [[ "$count" -ne 1 ]]; then
    echo "Error: ambiguous ${extension} artifacts for ${package} sha=${sha} in ${base_url}/" >&2
    printf '%s\n' "$matches" >&2
    return 1
  fi
  ra_url_escape "$(printf '%s\n' "$matches" | sed '/^$/d')"
}

ra_resolve_repo_artifact() {
  local repo_type="$1" distr="$2" arch="$3" package="$4" sha="$5" extension="$6"
  local repo_ctx=() base_url pkg_fmt deb_repo_root
  while IFS= read -r line; do
    repo_ctx+=("$line")
  done < <(ra_repo_context "$repo_type" "$distr" "$arch")
  base_url="${repo_ctx[0]:-}"
  pkg_fmt="${repo_ctx[1]:-}"
  deb_repo_root="${repo_ctx[2]:-}"

  if [[ "$pkg_fmt" == deb ]]; then
    ra_resolve_deb_artifact_by_sha "$repo_type" "$distr" "$arch" "$package" "$sha" "$extension" "$deb_repo_root"
  else
    ra_resolve_listing_artifact_by_sha "$base_url" "$pkg_fmt" "$arch" "$package" "$sha" "$extension"
  fi
}
