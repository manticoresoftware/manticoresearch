#!/usr/bin/env python3

import argparse
import os
import shutil
from pathlib import Path


def manifest(cache_dir: Path) -> dict[str, tuple[int, int]]:
    entries: dict[str, tuple[int, int]] = {}
    for path in sorted(cache_dir.rglob("*")):
        if path.is_file() and not path.is_symlink():
            stat = path.stat()
            entries[str(path.relative_to(cache_dir))] = (stat.st_size, stat.st_mtime_ns)
    return entries


def write_manifest(path: Path, entries: dict[str, tuple[int, int]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        "".join(f"{name}\t{size}\t{mtime_ns}\n" for name, (size, mtime_ns) in sorted(entries.items())),
        encoding="utf-8",
    )


def read_manifest(path: Path) -> dict[str, tuple[int, int]]:
    if not path.exists():
        return {}
    entries: dict[str, tuple[int, int]] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        name, size, mtime_ns = line.rsplit("\t", 2)
        entries[name] = (int(size), int(mtime_ns))
    return entries


def changed_roots(before: dict[str, tuple[int, int]], after: dict[str, tuple[int, int]]) -> list[str]:
    changed = {
        name
        for name in set(before) | set(after)
        if before.get(name) != after.get(name)
    }
    return sorted({name.split("/", 1)[0] for name in changed})


def stage(cache_dir: Path, destination: Path, roots: list[str]) -> None:
    if destination.exists():
        shutil.rmtree(destination)
    destination.mkdir(parents=True)
    (destination / ".clt-artifact-empty-marker").touch()
    for root in roots:
        source = cache_dir / root
        target = destination / root
        if source.is_dir():
            shutil.copytree(source, target, symlinks=True, dirs_exist_ok=True)
        elif source.exists() or source.is_symlink():
            shutil.copy2(source, target, follow_symlinks=False)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("snapshot", "stage"))
    parser.add_argument("--cache-dir", required=True, type=Path)
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--destination", type=Path)
    args = parser.parse_args()

    args.cache_dir.mkdir(parents=True, exist_ok=True)
    if args.command == "snapshot":
        write_manifest(args.manifest, manifest(args.cache_dir))
        return

    if args.destination is None:
        parser.error("--destination is required for stage")

    roots = changed_roots(read_manifest(args.manifest), manifest(args.cache_dir))
    stage(args.cache_dir, args.destination, roots)
    print("changed_roots=" + ",".join(roots))
    if os.environ.get("GITHUB_OUTPUT"):
        with open(os.environ["GITHUB_OUTPUT"], "a", encoding="utf-8") as output:
            output.write(f"changed={'true' if roots else 'false'}\n")


if __name__ == "__main__":
    main()
