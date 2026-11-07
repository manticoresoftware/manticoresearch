#!/usr/bin/env python3
"""Report per-test CLT failures from the last 30 days of main-branch CI."""

import csv
import io
import json
import os
import re
import subprocess
import zipfile
from collections import Counter, defaultdict
from datetime import datetime, timedelta, timezone
from pathlib import Path

DAYS = 30
MIN_EXECUTIONS = 5
SHARD = re.compile(r"clt-test-results-\d+-attempt-(\d+)/clt-test-results\.jsonl$")
FIELDS = ("test", "passed", "failed", "executions", "failure_rate", "last_failure_url")


def api(path):
    """Use the runner's authenticated GitHub CLI (also follows artifact redirects)."""
    return subprocess.check_output(["gh", "api", path])


def pages(path, field):
    for page in range(1, 1000):
        sep = "&" if "?" in path else "?"
        items = json.loads(api(f"{path}{sep}per_page=100&page={page}"))[field]
        yield from items
        if len(items) < 100:
            break


def count_results(stats, failures, run, archive, repo):
    with zipfile.ZipFile(io.BytesIO(archive)) as zipped:
        found = False
        for filename in zipped.namelist():
            match = SHARD.fullmatch(filename)
            if not match:
                continue
            found = True
            attempt = int(match[1])
            for line in zipped.read(filename).splitlines():
                if not line.strip():
                    continue
                record = json.loads(line)
                status = record["status"]
                if status not in ("passed", "failed"):
                    continue  # Missing/interrupted tests are not successes.
                test = record["test"]
                stats[test][status] += 1
                if status == "failed":
                    occurred = (run["created_at"], attempt)
                    if test not in failures or occurred > failures[test][0]:
                        url = f'https://github.com/{repo}/actions/runs/{run["id"]}/attempts/{attempt}'
                        failures[test] = (occurred, url)
        if not found:
            raise ValueError("No CLT shard results in merged artifact")


def make_rows(stats, failures):
    rows = []
    for test, counts in stats.items():
        passed, failed = counts["passed"], counts["failed"]
        rows.append({
            "test": test, "passed": passed, "failed": failed,
            "executions": passed + failed,
            "failure_rate": round(100 * failed / (passed + failed), 2),
            "last_failure_url": failures.get(test, (None, ""))[1],
        })
    return sorted(rows, key=lambda r: (-r["failure_rate"], -r["failed"], r["test"]))


def summary(rows, runs, reports):
    lines = [
        f"## CLT test failure rates — last {DAYS} days", "",
        f"**{reports}** runs with CLT results out of **{runs}** completed main-branch push runs.", "",
        "Failure rate = failed / (passed + failed). Only completed tests are counted; retries count as separate executions.", "",
    ]
    failing = [r for r in rows if r["failed"] and r["executions"] >= MIN_EXECUTIONS]
    if failing:
        lines.extend(("| Test | Failures | Executions | Failure rate | Last failure |",
                      "| --- | ---: | ---: | ---: | --- |"))
        for r in failing[:50]:
            test = r["test"].replace("|", "\\|")
            lines.append(f'| `{test}` | {r["failed"]} | {r["executions"]} | '
                         f'{r["failure_rate"]:.1f}% | [Run]({r["last_failure_url"]}) |')
    else:
        lines.append("No CLT results available for this period." if not reports
                     else f"No failing tests with at least {MIN_EXECUTIONS} executions yet.")
    lines.extend(("", "Download the CSV artifact for all tests, including those with no failures.", ""))
    return "\n".join(lines)


def main():
    repo = os.environ["GITHUB_REPOSITORY"]
    cutoff = datetime.now(timezone.utc) - timedelta(days=DAYS)
    stats, failures = defaultdict(Counter), {}
    runs = reports = 0
    for run in pages(f"repos/{repo}/actions/workflows/test.yml/runs?branch=main&event=push", "workflow_runs"):
        if datetime.fromisoformat(run["created_at"].replace("Z", "+00:00")) < cutoff:
            break
        if run["status"] != "completed":
            continue
        runs += 1
        artifacts = [a for a in pages(f'repos/{repo}/actions/runs/{run["id"]}/artifacts', "artifacts")
                     if re.fullmatch(r"clt-results-merged-\d+", a["name"]) and not a["expired"]]
        if artifacts:
            newest = max(artifacts, key=lambda a: int(a["name"].rsplit("-", 1)[1]))
            count_results(stats, failures, run, api(f'repos/{repo}/actions/artifacts/{newest["id"]}/zip'), repo)
            reports += 1

    rows = make_rows(stats, failures)
    csv_path = Path(os.environ["RUNNER_TEMP"]) / "clt-failure-rates.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=FIELDS)
        writer.writeheader()
        writer.writerows(rows)
    text = summary(rows, runs, reports)
    with Path(os.environ["GITHUB_STEP_SUMMARY"]).open("a", encoding="utf-8") as handle:
        handle.write(text)
    print(text)


if __name__ == "__main__":
    main()
