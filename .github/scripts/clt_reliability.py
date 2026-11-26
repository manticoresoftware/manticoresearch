#!/usr/bin/env python3
"""Report per-test CLT failures from the last 30 days of main-branch CI."""

import csv
import io
import json
import os
import re
import subprocess
import zipfile
from collections import defaultdict
from datetime import datetime, timedelta, timezone
from pathlib import Path

DAYS = 30
MIN_EXECUTIONS = 5
SHARD = re.compile(r"clt-test-results-\d+-attempt-(\d+)/clt-test-results\.jsonl$")
FIELDS = ("test", "passed", "failed", "executions", "failure_rate", "last_failure_url",
          "last_failure_at", "passed_since_last_failure")


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


def parse_timestamp(value):
    return datetime.fromisoformat(value.replace("Z", "+00:00"))


def count_results(results, run, archive, repo, artifact_times):
    with zipfile.ZipFile(io.BytesIO(archive)) as zipped:
        found = False
        for filename in zipped.namelist():
            match = SHARD.fullmatch(filename)
            if not match:
                continue
            found = True
            attempt = int(match[1])
            shard = filename.split("/", 1)[0]
            # Shard uploads approximate test completion and preserve retry chronology.
            # Older shard artifacts may have been removed while the merged ZIP remains.
            timestamp = (artifact_times.get(shard)
                         or artifact_times.get(f"clt-results-merged-{attempt}")
                         or run["created_at"])
            occurred = (parse_timestamp(timestamp), run["id"], attempt)
            url = f'https://github.com/{repo}/actions/runs/{run["id"]}/attempts/{attempt}'
            for line in zipped.read(filename).splitlines():
                if not line.strip():
                    continue
                record = json.loads(line)
                status = record["status"]
                if status not in ("passed", "failed"):
                    continue  # Missing/interrupted tests are not successes.
                results[record["test"]].append((occurred, status, url))
        if not found:
            raise ValueError("No CLT shard results in merged artifact")


def make_rows(results):
    rows = []
    for test, executions in results.items():
        failed = sum(status == "failed" for _, status, _ in executions)
        passed = len(executions) - failed
        last_failure = max((entry for entry in executions if entry[1] == "failed"), default=None)
        rows.append({
            "test": test, "passed": passed, "failed": failed,
            "executions": len(executions),
            "failure_rate": round(100 * failed / len(executions), 2),
            "last_failure_url": last_failure[2] if last_failure else "",
            "last_failure_at": last_failure[0][0].strftime("%Y-%m-%dT%H:%M:%SZ") if last_failure else "",
            "passed_since_last_failure": (
                sum(status == "passed" and occurred > last_failure[0] for occurred, status, _ in executions)
                if last_failure else ""
            ),
        })
    return sorted(rows, key=lambda r: (-r["failure_rate"], -r["failed"], r["test"]))


def relative_age(timestamp, now):
    minutes = max(0, int((now - timestamp).total_seconds() // 60))
    days, remaining = divmod(minutes, 24 * 60)
    hours, minutes = divmod(remaining, 60)
    if days:
        return f"{days}d" + (f" {hours}h" if hours else "") + " ago"
    if hours:
        return f"{hours}h" + (f" {minutes}m" if minutes else "") + " ago"
    return f"{minutes}m ago" if minutes else "just now"


def summary(rows, runs, reports, now):
    lines = [
        f"## CLT test failure rates — last {DAYS} days", "",
        f"Generated at {now:%Y-%m-%d %H:%M UTC}.", "",
        f"**{reports}** runs with CLT results out of **{runs}** completed main-branch push runs.", "",
        "Failure rate = failed / (passed + failed). Only completed tests are counted; retries count as separate executions.",
        "Last failure is relative to report generation, using shard artifact upload time (or run metadata if unavailable). "
        "Passed since counts successful executions after that failure within the reporting window.", "",
    ]
    failing = [r for r in rows if r["failed"] and r["executions"] >= MIN_EXECUTIONS]
    if failing:
        lines.extend(("| Test | Failures | Executions | Failure rate | Last failure | Passed since |",
                      "| --- | ---: | ---: | ---: | --- | ---: |"))
        for r in failing[:50]:
            test = r["test"].replace("|", "\\|")
            age = relative_age(parse_timestamp(r["last_failure_at"]), now)
            lines.append(f'| `{test}` | {r["failed"]} | {r["executions"]} | '
                         f'{r["failure_rate"]:.1f}% | [{age}]({r["last_failure_url"]}) | '
                         f'{r["passed_since_last_failure"]} |')
    else:
        lines.append("No CLT results available for this period." if not reports
                     else f"No failing tests with at least {MIN_EXECUTIONS} executions yet.")
    lines.extend(("", "Download the CSV artifact for all tests, including those with no failures.", ""))
    return "\n".join(lines)


def main():
    repo = os.environ["GITHUB_REPOSITORY"]
    cutoff = datetime.now(timezone.utc) - timedelta(days=DAYS)
    results = defaultdict(list)
    runs = reports = 0
    for run in pages(f"repos/{repo}/actions/workflows/test.yml/runs?branch=main&event=push", "workflow_runs"):
        if parse_timestamp(run["created_at"]) < cutoff:
            break
        if run["status"] != "completed":
            continue
        runs += 1
        artifacts = list(pages(f'repos/{repo}/actions/runs/{run["id"]}/artifacts', "artifacts"))
        merged = [a for a in artifacts
                  if re.fullmatch(r"clt-results-merged-\d+", a["name"]) and not a["expired"]]
        if merged:
            newest = max(merged, key=lambda a: int(a["name"].rsplit("-", 1)[1]))
            artifact_times = {a["name"]: a["created_at"] for a in artifacts}
            count_results(results, run, api(f'repos/{repo}/actions/artifacts/{newest["id"]}/zip'),
                          repo, artifact_times)
            reports += 1

    rows = make_rows(results)
    csv_path = Path(os.environ["RUNNER_TEMP"]) / "clt-failure-rates.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=FIELDS)
        writer.writeheader()
        writer.writerows(rows)
    text = summary(rows, runs, reports, datetime.now(timezone.utc))
    with Path(os.environ["GITHUB_STEP_SUMMARY"]).open("a", encoding="utf-8") as handle:
        handle.write(text)
    print(text)


if __name__ == "__main__":
    main()
