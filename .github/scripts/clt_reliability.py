#!/usr/bin/env python3
"""Report CLT failure rates from the last 30 days of main-branch CI."""

import csv
import io
import json
import os
import re
import urllib.request
import zipfile
from collections import Counter, defaultdict
from datetime import datetime, timedelta, timezone
from pathlib import Path
from urllib.parse import urlsplit

DAYS = 30
MIN_EXECUTIONS = 5
SHARD = re.compile(r"clt-test-results-\d+-attempt-(\d+)/clt-test-results\.jsonl$")


class SafeRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        result = super().redirect_request(req, fp, code, msg, headers, newurl)
        if result and urlsplit(req.full_url).netloc != urlsplit(newurl).netloc:
            result.remove_header("Authorization")
        return result


def github_get(url):
    request = urllib.request.Request(url, headers={
        "Authorization": f'Bearer {os.environ["GITHUB_TOKEN"]}',
        "Accept": "application/vnd.github+json",
        "User-Agent": "manticore-clt-reliability",
    })
    with urllib.request.build_opener(SafeRedirect()).open(request, timeout=45) as response:
        return response.read()


def pages(url, key):
    for page in range(1, 1000):
        separator = "&" if "?" in url else "?"
        data = json.loads(github_get(f"{url}{separator}per_page=100&page={page}"))[key]
        yield from data
        if len(data) < 100:
            break


def shard_results(archive):
    """Each shard/attempt has its own file, so retries are distinct executions."""
    with zipfile.ZipFile(io.BytesIO(archive)) as zipped:
        found = False
        for filename in zipped.namelist():
            match = SHARD.fullmatch(filename)
            if match:
                found = True
                for line in zipped.read(filename).splitlines():
                    if line.strip():
                        yield int(match[1]), json.loads(line)
        if not found:
            raise ValueError("No CLT shard results in merged artifact")


def count_results(stats, last_failures, run, archive, repository):
    for attempt, record in shard_results(archive):
        if record["status"] not in ("passed", "failed"):
            continue  # Missing or interrupted tests did not finish.
        name = record["test"]
        stats[name][record["status"]] += 1
        if record["status"] == "failed":
            failed_at = (run["created_at"], attempt)
            if name not in last_failures or failed_at > last_failures[name][0]:
                url = f'https://github.com/{repository}/actions/runs/{run["id"]}/attempts/{attempt}'
                last_failures[name] = (failed_at, url)


def make_rows(stats, last_failures):
    rows = []
    for name, counts in stats.items():
        passed, failed = counts["passed"], counts["failed"]
        total = passed + failed
        rows.append({
            "test": name, "passed": passed, "failed": failed,
            "executions": total, "failure_rate": round(100 * failed / total, 2),
            "last_failure_url": last_failures.get(name, (None, ""))[1],
        })
    return sorted(rows, key=lambda row: (-row["failure_rate"], -row["failed"], row["test"]))


def summary(rows, runs, reports):
    lines = [
        f"## CLT test failure rates — last {DAYS} days", "",
        f"**{reports}** runs with CLT results out of **{runs}** completed main-branch push runs.", "",
        "Failure rate = failed / (passed + failed). Only completed tests are counted; "
        "retries count as separate executions.", "",
    ]
    failing = [row for row in rows if row["failed"] and row["executions"] >= MIN_EXECUTIONS]
    if failing:
        lines += ["| Test | Failures | Executions | Failure rate | Last failure |",
                  "| --- | ---: | ---: | ---: | --- |"]
        for row in failing[:50]:
            name = row["test"].replace("|", "\\|")
            lines.append(f'| `{name}` | {row["failed"]} | {row["executions"]} | '
                         f'{row["failure_rate"]:.1f}% | [Run]({row["last_failure_url"]}) |')
    else:
        lines.append(f"No failing tests with at least {MIN_EXECUTIONS} executions yet.")
    lines += ["", "Download the CSV artifact for all tests, including those with no failures.", ""]
    return "\n".join(lines)


def main():
    repository = os.environ["GITHUB_REPOSITORY"]
    base = f"https://api.github.com/repos/{repository}"
    cutoff = datetime.now(timezone.utc) - timedelta(days=DAYS)
    stats, last_failures = defaultdict(Counter), {}
    runs = reports = 0
    for run in pages(f"{base}/actions/workflows/test.yml/runs?branch=main&event=push", "workflow_runs"):
        if datetime.fromisoformat(run["created_at"].replace("Z", "+00:00")) < cutoff:
            break  # Runs are newest first.
        if run["status"] != "completed":
            continue
        runs += 1
        artifacts = [
            artifact for artifact in pages(f'{base}/actions/runs/{run["id"]}/artifacts', "artifacts")
            if re.fullmatch(r"clt-results-merged-\d+", artifact["name"]) and not artifact["expired"]
        ]
        if not artifacts:
            continue
        latest = max(artifacts, key=lambda artifact: int(artifact["name"].rsplit("-", 1)[1]))
        count_results(stats, last_failures, run, github_get(latest["archive_download_url"]), repository)
        reports += 1

    rows = make_rows(stats, last_failures)
    output = Path(os.environ["RUNNER_TEMP"]) / "clt-failure-rates.csv"
    with output.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=[
            "test", "passed", "failed", "executions", "failure_rate", "last_failure_url",
        ])
        writer.writeheader()
        writer.writerows(rows)
    message = summary(rows, runs, reports)
    with Path(os.environ["GITHUB_STEP_SUMMARY"]).open("a", encoding="utf-8") as handle:
        handle.write(message)
    print(message)


if __name__ == "__main__":
    main()
