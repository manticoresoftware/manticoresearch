#!/usr/bin/env python3
import argparse
import html
import re
import sys
import urllib.request
from urllib.parse import urljoin


def norm_arch(distr: str, arch: str) -> str:
    if distr == "windows":
        return "x64"
    if arch in ("x86_64", "amd64", "x64"):
        return "amd64" if distr not in ("rhel8", "rhel9", "rhel10", "windows", "macos") else "x86_64"
    if arch in ("aarch64", "arm64"):
        return "arm64" if distr not in ("rhel8", "rhel9", "rhel10", "windows", "macos") else "aarch64"
    return arch


def repo_index_url(repo_type: str, distr: str, arch: str) -> str:
    arch = norm_arch(distr, arch)
    if distr == "windows":
        return f"https://repo.manticoresearch.com/repository/manticoresearch_windows/{repo_type}/x64/"
    if distr == "macos":
        return f"https://repo.manticoresearch.com/repository/manticoresearch_macos/{repo_type}/"
    if distr.startswith("rhel"):
        ver = distr[4:]
        return f"https://repo.manticoresearch.com/repository/manticoresearch/{repo_type}/centos/{ver}/{arch}/"
    return f"https://repo.manticoresearch.com/repository/manticoresearch_{distr}_{repo_type}/dists/{distr}/main/binary-{arch}/"


def find_candidates(index_html: str, package: str, sha: str, extensions: list[str]) -> list[str]:
    hrefs = re.findall(r'href="([^"]+)"', index_html, re.IGNORECASE)
    sha = sha.lower()
    matches = []
    for href in hrefs:
        name = html.unescape(href)
        lname = name.lower()
        if package.lower() not in lname:
            continue
        if sha not in lname:
            continue
        if not any(lname.endswith(ext.lower()) for ext in extensions):
            continue
        matches.append(name)
    return matches


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo-type", default="dev")
    ap.add_argument("--distr", required=True)
    ap.add_argument("--arch", required=True)
    ap.add_argument("--package", default="manticore-columnar-lib")
    ap.add_argument("--sha", required=True)
    ap.add_argument("--extensions", nargs="+", default=[".deb", ".rpm", ".tar.gz", ".zip"])
    args = ap.parse_args()

    index_url = repo_index_url(args.repo_type, args.distr, args.arch)
    try:
        with urllib.request.urlopen(index_url, timeout=30) as resp:
            body = resp.read().decode("utf-8", errors="replace")
    except Exception as e:
        print(f"failed to fetch package index {index_url}: {e}", file=sys.stderr)
        return 2

    candidates = find_candidates(body, args.package, args.sha, args.extensions)
    if not candidates:
        print(f"no package found for {args.package} sha={args.sha} in {index_url}", file=sys.stderr)
        return 1

    candidates.sort()
    print(urljoin(index_url, candidates[-1]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
