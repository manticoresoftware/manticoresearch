#!/usr/bin/env python3
"""Collect CLT test outcomes and produce a per-test CI reliability report.

No third-party dependencies: the report workflow runs on a stock GitHub runner.
"""

import argparse
import csv
import io
import json
import os
import sys
import urllib.parse
import urllib.request
import zipfile
from collections import Counter, defaultdict
from datetime import datetime, timedelta, timezone
from pathlib import Path

STATUSES = frozenset({"passed", "failed", "interrupted", "missing", "not_run"})


def utc_now():
    return datetime.now(timezone.utc)


def parse_date(text):
    return datetime.fromisoformat(text.replace("Z", "+00:00"))


def write_json(path, data):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def normalize_test(name):
    if not isinstance(name, str) or not name.strip():
        raise ValueError(f"Invalid test path: {name!r}")
    return name if name.endswith(".rec") else name + ".rec"


def merge_shards(root, metadata, planned_shards=None):
    """Create exactly one record for every scheduled test, including those never run."""
    manifests = sorted(Path(root).glob("clt-test-results-*/manifest.json"))
    if not manifests and not planned_shards:
        raise ValueError(f"No CLT shard manifests found under {root}")
    paths_by_shard = {}
    for path in manifests:
        shard = str(json.loads(path.read_text(encoding="utf-8"))["shard"])
        if shard in paths_by_shard:
            raise ValueError(f"Duplicate shard manifest {shard}")
        paths_by_shard[shard] = path

    if planned_shards is not None:
        expected_shards = {str(shard["id"]): shard["tests"] for shard in planned_shards}
        if set(paths_by_shard) - set(expected_shards):
            raise ValueError("Shard reports contain an unknown shard")
    else:
        expected_shards = {shard: None for shard in paths_by_shard}

    tests = []
    seen_tests = set()
    shards = []
    for shard, planned_tests in expected_shards.items():
        path = paths_by_shard.get(shard)
        if path:
            manifest = json.loads(path.read_text(encoding="utf-8"))
        else:
            manifest = {"shard": shard, "attempt": metadata["attempt"], "tests": planned_tests}
        source_attempt = int(manifest["attempt"])
        expected = [normalize_test(t) for t in manifest["tests"]]
        if planned_tests is not None and set(expected) != {normalize_test(t) for t in planned_tests}:
            raise ValueError(f"Shard {shard} doesn't match the scheduled test matrix")
        if len(set(expected)) != len(expected):
            raise ValueError(f"Duplicate scheduled tests in {path}")
        reported = {}
        outcomes_path = path.with_name("results.jsonl") if path else None
        if outcomes_path and outcomes_path.exists():
            for line_no, line in enumerate(outcomes_path.read_text(encoding="utf-8").splitlines(), 1):
                if not line.strip():
                    continue
                result = json.loads(line)
                test = normalize_test(result["test"])
                status = result["status"]
                if status not in STATUSES - {"not_run"}:
                    raise ValueError(f"Unexpected CLT status {status!r} in {outcomes_path}:{line_no}")
                if test in reported:
                    raise ValueError(f"Duplicate outcome for {test} in {outcomes_path}")
                reported[test] = result

        extra = set(reported) - set(expected)
        if extra:
            raise ValueError(f"Unscheduled CLT results in {outcomes_path}: {sorted(extra)}")
        for test in expected:
            if test in seen_tests:
                raise ValueError(f"Test {test} scheduled more than once")
            seen_tests.add(test)
            result = reported.get(test, {"status": "not_run", "exit_code": None, "duration_ms": None})
            tests.append({
                "test": test,
                "status": result["status"],
                "exit_code": result.get("exit_code"),
                "duration_ms": result.get("duration_ms"),
                "shard": shard,
                "source_attempt": source_attempt,
            })
        shards.append({
            "shard": shard, "source_attempt": source_attempt,
            "scheduled": len(expected), "reported": len(reported),
        })

    return {"schema_version": 1, **metadata, "shards": shards, "tests": tests}


def run_summary(report):
    counts = Counter(r["status"] for r in report["tests"])
    url = f'https://github.com/{report["repository"]}/actions/runs/{report["run_id"]}'
    lines = [
        "## CLT per-test results",
        "",
        f'**{len(report["tests"])} scheduled** · '
        f'{counts["passed"]} passed · {counts["failed"]} failed · '
        f'{counts["interrupted"]} interrupted · {counts["missing"]} missing · '
        f'{counts["not_run"]} not run',
        "",
        f'[Workflow run]({url}) · Detailed per-test JSON is attached as a workflow artifact.',
        "",
    ]
    bad = [r for r in report["tests"] if r["status"] not in ("passed", "not_run")]
    if bad:
        lines += ["| CLT test | Result | Shard |", "| --- | --- | --- |"]
        for result in bad[:50]:
            name = result["test"].replace("|", "\\|")
            lines.append(f'| `{name}` | {result["status"]} | {result["shard"]} |')
        if len(bad) > 50:
            lines.append(f"\n{len(bad) - 50} additional failures are in the artifact.")
    if counts["not_run"]:
        lines.append("\nNot-run tests are excluded from failure rates (e.g. a shard stopped early).")
    return "\n".join(lines) + "\n"


class StripCrossHostAuthorization(urllib.request.HTTPRedirectHandler):
    """Do not forward the API token to GitHub's signed artifact storage URL."""

    def redirect_request(self, req, fp, code, msg, headers, newurl):
        redirected = super().redirect_request(req, fp, code, msg, headers, newurl)
        if redirected and urllib.parse.urlsplit(req.full_url).netloc != urllib.parse.urlsplit(newurl).netloc:
            redirected.headers.pop("Authorization", None)
            redirected.unredirected_hdrs.pop("Authorization", None)
        return redirected


class GitHubArtifacts:
    def __init__(self, repository, token):
        self.base = f"https://api.github.com/repos/{repository}"
        self.token = token
        self.opener = urllib.request.build_opener(StripCrossHostAuthorization())

    def get_bytes(self, path):
        url = path if path.startswith("https://") else self.base + path
        request = urllib.request.Request(url, headers={
            "Accept": "application/vnd.github+json",
            "Authorization": f"Bearer {self.token}",
            "X-GitHub-Api-Version": "2022-11-28",
            "User-Agent": "manticore-clt-reliability",
        })
        with self.opener.open(request, timeout=45) as response:
            return response.read()

    def get_json(self, path):
        return json.loads(self.get_bytes(path))

    def paginated(self, path, key):
        page = 1
        while True:
            separator = "&" if "?" in path else "?"
            payload = self.get_json(f"{path}{separator}per_page=100&page={page}")
            items = payload[key]
            yield from items
            if len(items) < 100:
                break
            page += 1

    def read_report_artifact(self, artifact):
        archive = self.get_bytes(artifact["archive_download_url"])
        with zipfile.ZipFile(io.BytesIO(archive)) as bundle:
            names = [n for n in bundle.namelist() if n.endswith("clt-results.json")]
            if len(names) != 1:
                raise ValueError(f"Expected one clt-results.json, found {names}")
            return json.loads(bundle.read(names[0]))


def collect_runs(client, branch, days, now):
    since = now - timedelta(days=days)
    reports = []
    errors = []
    run_count = 0
    for run in client.paginated(f"/actions/workflows/test.yml/runs?branch={urllib.parse.quote(branch)}&event=push", "workflow_runs"):
        if parse_date(run["created_at"]) < since:
            break  # GitHub returns most recent runs first.
        run_count += 1
        if run["status"] != "completed":
            continue
        for artifact in client.paginated(f'/actions/runs/{run["id"]}/artifacts', "artifacts"):
            if not artifact["name"].startswith("clt-results-merged-") or artifact["expired"]:
                continue
            try:
                report = client.read_report_artifact(artifact)
                if (report["run_id"] != run["id"] or report["event"] != "push"
                        or report["ref"] != f"refs/heads/{branch}" or report["schema_version"] != 1):
                    raise ValueError("Artifact metadata doesn't match the workflow run")
                reports.append(report)
            except (ValueError, KeyError, OSError, zipfile.BadZipFile, json.JSONDecodeError) as exc:
                errors.append(f'{run["id"]}/{artifact["id"]}: {exc}')
    return reports, run_count, errors


def compute_statistics(reports, repository):
    results = defaultdict(lambda: Counter())
    latest_failure = {}
    observed_executions = set()
    for report in reports:
        run_id = report["run_id"]
        for record in report["tests"]:
            name = normalize_test(record["test"])
            # "Re-run failed jobs" keeps the prior successful shard artifacts. Avoid
            # treating those as new executions on every workflow attempt.
            key = (run_id, str(record["shard"]), record["source_attempt"], name)
            if key in observed_executions:
                continue
            observed_executions.add(key)
            status = record["status"]
            if status not in STATUSES:
                raise ValueError(f"Unexpected status: {status}")
            results[name][status] += 1
            if status in ("failed", "interrupted"):
                attempt = record["source_attempt"]
                timestamp = parse_date(report["created_at"])
                if name not in latest_failure or timestamp > latest_failure[name][0]:
                    latest_failure[name] = (timestamp, f'https://github.com/{repository}/actions/runs/{run_id}/attempts/{attempt}')

    rows = []
    for name, counts in results.items():
        passed, failed = counts["passed"], counts["failed"]
        executed = passed + failed
        rows.append({
            "test": name,
            "executed": executed,
            "passed": passed,
            "failed": failed,
            "failure_rate": round(100 * failed / executed, 2) if executed else None,
            "interrupted": counts["interrupted"],
            "missing": counts["missing"],
            "not_run": counts["not_run"],
            "flaky": passed > 0 and failed > 0,
            "last_failure": latest_failure[name][0].isoformat() if name in latest_failure else "",
            "last_failure_url": latest_failure[name][1] if name in latest_failure else "",
        })
    rows.sort(key=lambda r: (-(r["failure_rate"] or 0), -r["failed"], -r["executed"], r["test"]))
    return rows


def failure_summary(rows, days, run_count, report_count, errors):
    observed = [r for r in rows if r["executed"]]
    reliable = [r for r in observed if r["executed"] >= 5 and r["failed"] > 0]
    low_sample = [r for r in observed if r["executed"] < 5 and r["failed"] > 0]
    lines = [
        f"## CLT per-test failure rates — last {days} days",
        "",
        f"Scanned **{run_count}** main-branch push runs, **{report_count}** CLT result artifacts, "
        f"**{len(observed)}** distinct tests with executions.",
        "",
        "Failure rate = failed / (passed + failed). Interrupted, missing, and not-run "
        "tests are tracked separately and are **not** counted as successful runs. "
        "Only actual shard executions are counted when failed jobs are rerun.",
        "",
    ]
    if errors:
        lines.extend([f"⚠️ Unable to read {len(errors)} artifacts; the report may be incomplete.", ""])
    if reliable:
        lines.extend(["### Highest failure rates (at least 5 executions)", "",
                      "| Test | Failures | Runs | Failure rate | Interrupted | Last failing run |",
                      "| --- | ---: | ---: | ---: | ---: | --- |"])
        for r in reliable[:50]:
            test = r["test"].replace("|", "\\|")
            link = f'[View]({r["last_failure_url"]})' if r["last_failure_url"] else "—"
            lines.append(f'| `{test}` | {r["failed"]} | {r["executed"]} | {r["failure_rate"]:.1f}% | {r["interrupted"]} | {link} |')
        lines.append("")
    else:
        lines.extend(["No failing tests with at least five executions yet.", ""])
    if low_sample:
        lines.append(f"**{len(low_sample)}** additional failing tests have fewer than five executions; see the CSV artifact.")
        lines.append("")
    if not observed:
        lines.extend(["No per-test data yet. Statistics begin collecting after the CLT reporting changes are merged.", ""])
    lines.append("Download the CSV / JSON artifact for all tests, including tests with 0% failures.")
    return "\n".join(lines) + "\n"


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    merge = commands.add_parser("merge", help="Merge per-shard results into one run artifact")
    merge.add_argument("--input", required=True)
    merge.add_argument("--output", required=True)
    merge.add_argument("--summary", required=True)
    report = commands.add_parser("report", help="Read recent run artifacts and report failure rates")
    report.add_argument("--repository", required=True)
    report.add_argument("--branch", default="main")
    report.add_argument("--days", type=int, default=30)
    report.add_argument("--output", required=True)
    report.add_argument("--summary", required=True)
    args = parser.parse_args(argv)
    if args.command == "merge":
        metadata = {
            "repository": os.environ["CLT_REPO"], "run_id": int(os.environ["CLT_RUN_ID"]),
            "attempt": int(os.environ["CLT_ATTEMPT"]), "event": os.environ["CLT_EVENT"],
            "ref": os.environ["CLT_REF"], "head_sha": os.environ["CLT_SHA"],
            "created_at": utc_now().isoformat(),
        }
        matrix = json.loads(os.environ["CLT_MATRIX_JSON"])
        merged = merge_shards(args.input, metadata, planned_shards=matrix["chunk"])
        write_json(args.output, merged)
        Path(args.summary).open("a", encoding="utf-8").write(run_summary(merged))
        return 0

    if args.days <= 0:
        parser.error("--days must be positive")
    token = os.environ["GITHUB_TOKEN"]
    reports, run_count, errors = collect_runs(GitHubArtifacts(args.repository, token), args.branch, args.days, utc_now())
    rows = compute_statistics(reports, args.repository)
    output = Path(args.output)
    output.mkdir(parents=True, exist_ok=True)
    write_json(output / "clt-failure-rates.json", {"days": args.days, "runs": run_count, "artifacts": len(reports), "tests": rows, "errors": errors})
    with (output / "clt-failure-rates.csv").open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=[
            "test", "executed", "passed", "failed", "failure_rate", "interrupted", "missing",
            "not_run", "flaky", "last_failure", "last_failure_url",
        ])
        writer.writeheader()
        writer.writerows(rows)
    summary = failure_summary(rows, args.days, run_count, len(reports), errors)
    Path(args.summary).open("a", encoding="utf-8").write(summary)
    print(summary)
    for error in errors:
        print(f"WARNING: {error}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
