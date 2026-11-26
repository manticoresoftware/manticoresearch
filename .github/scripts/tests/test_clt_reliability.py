"""Checks for CLT failure-rate accounting and report rendering."""

import importlib.util
import io
import json
import unittest
import zipfile
from collections import defaultdict
from datetime import timedelta
from pathlib import Path
from unittest.mock import patch

spec = importlib.util.spec_from_file_location(
    "clt_reliability", Path(__file__).resolve().parents[1] / "clt_reliability.py"
)
clt = importlib.util.module_from_spec(spec)
spec.loader.exec_module(clt)


def archive(shards):
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w") as zipped:
        for shard, attempt, results in shards:
            zipped.writestr(
                f"clt-test-results-{shard}-attempt-{attempt}/clt-test-results.jsonl",
                "".join(json.dumps(record) + "\n" for record in results),
            )
    return buffer.getvalue()


class ReliabilityTests(unittest.TestCase):
    def setUp(self):
        self.results = defaultdict(list)
        self.run = {"id": 42, "created_at": "2026-10-22T12:00:00Z"}
        self.now = clt.parse_timestamp("2026-10-26T12:30:00Z")

    def collect(self, shards, artifact_times=None, run=None):
        clt.count_results(self.results, run or self.run, archive(shards), "org/repo", artifact_times or {})
        return {r["test"]: r for r in clt.make_rows(self.results)}

    def test_only_reexecuted_shards_count_on_retry(self):
        rows = self.collect([
            (1, 1, [{"test": "a.rec", "status": "passed"}]),
            (2, 1, [{"test": "b.rec", "status": "failed"}]),
            (2, 2, [{"test": "b.rec", "status": "passed"}]),
        ])
        self.assertEqual(rows["a.rec"]["executions"], 1)
        self.assertEqual((rows["b.rec"]["failed"], rows["b.rec"]["passed"]), (1, 1))
        self.assertEqual(rows["b.rec"]["failure_rate"], 50)
        self.assertTrue(rows["b.rec"]["last_failure_url"].endswith("/attempts/1"))
        self.assertEqual(rows["b.rec"]["passed_since_last_failure"], 1)
        self.assertEqual(rows["a.rec"]["last_failure_at"], "")
        self.assertEqual(rows["a.rec"]["passed_since_last_failure"], "")

    def test_missing_and_interrupted_are_not_passes(self):
        rows = self.collect([(1, 1, [
            {"test": "ok.rec", "status": "passed"},
            {"test": "missing.rec", "status": "missing"},
            {"test": "interrupted.rec", "status": "interrupted"},
        ])])
        self.assertEqual(list(rows), ["ok.rec"])

    def test_latest_failure_link(self):
        rows = self.collect([
            (1, 1, [{"test": "a.rec", "status": "failed"}]),
            (1, 2, [{"test": "a.rec", "status": "failed"}]),
        ])
        self.assertEqual(rows["a.rec"]["executions"], 2)
        self.assertTrue(rows["a.rec"]["last_failure_url"].endswith("/attempts/2"))

    def test_no_shard_results_is_an_error(self):
        with self.assertRaisesRegex(ValueError, "No CLT shard results"):
            self.collect([])

    def test_empty_reports_and_clean_runs_are_distinct(self):
        self.assertIn("No CLT results available", clt.summary([], 2, 0, self.now))
        self.assertIn("No failing tests", clt.summary([], 2, 1, self.now))

    def test_retry_of_older_run_can_be_latest_failure(self):
        test = "a.rec"
        self.collect([(1, 1, [{"test": test, "status": "failed"}])],
                     {"clt-test-results-1-attempt-1": "2026-10-24T12:00:00Z"},
                     {"id": 43, "created_at": "2026-10-24T11:00:00Z"})
        self.collect([(1, 1, [{"test": test, "status": "passed"}])],
                     {"clt-test-results-1-attempt-1": "2026-10-25T12:00:00Z"},
                     {"id": 44, "created_at": "2026-10-25T11:00:00Z"})
        rows = self.collect([
            (1, 3, [{"test": test, "status": "passed"}]),
            (1, 2, [{"test": test, "status": "failed"}]),
            (1, 1, [{"test": test, "status": "passed"}]),
        ], {
            "clt-test-results-1-attempt-1": "2026-10-22T13:00:00Z",
            "clt-test-results-1-attempt-2": "2026-10-25T14:00:00Z",
            "clt-test-results-1-attempt-3": "2026-10-25T15:00:00Z",
            "clt-results-merged-3": "2026-10-25T16:00:00Z",
        })
        row = rows[test]
        self.assertEqual((row["failed"], row["passed"]), (2, 3))
        self.assertEqual(row["last_failure_at"], "2026-10-25T14:00:00Z")
        self.assertTrue(row["last_failure_url"].endswith("/42/attempts/2"))
        self.assertEqual(row["passed_since_last_failure"], 1)
        report = clt.summary([row], 3, 3, self.now)
        self.assertIn("Generated at 2026-10-26 12:30 UTC", report)
        self.assertIn("| 40.0% | [22h 30m ago](https://github.com/org/repo/actions/runs/42/attempts/2) | 1 |", report)

    def test_missing_shard_timestamp_falls_back_to_its_merged_attempt(self):
        rows = self.collect([
            (1, 2, [{"test": "a.rec", "status": "failed"}]),
            (1, 3, [{"test": "a.rec", "status": "passed"}]),
        ], {
            "clt-results-merged-2": "2026-10-23T12:00:00Z",
            "clt-results-merged-3": "2026-10-24T12:00:00Z",
        })
        self.assertEqual(rows["a.rec"]["last_failure_at"], "2026-10-23T12:00:00Z")
        self.assertEqual(rows["a.rec"]["passed_since_last_failure"], 1)

    def test_missing_all_artifact_timestamps_falls_back_to_run(self):
        rows = self.collect([(1, 1, [{"test": "a.rec", "status": "failed"}])])
        self.assertEqual(rows["a.rec"]["last_failure_at"], self.run["created_at"])
        self.assertEqual(rows["a.rec"]["passed_since_last_failure"], 0)

    def test_relative_age(self):
        for minutes, expected in [
            (-1, "just now"), (0, "just now"), (35, "35m ago"),
            (60, "1h ago"), (125, "2h 5m ago"), (1440, "1d ago"),
            (1440 * 2 + 60 * 5 + 20, "2d 5h ago"),
        ]:
            with self.subTest(minutes=minutes):
                self.assertEqual(clt.relative_age(self.now - timedelta(minutes=minutes), self.now), expected)

    @patch.object(clt.subprocess, "check_output", return_value=b"zip-data")
    def test_github_cli_gets_raw_bytes(self, command):
        self.assertEqual(clt.api("repos/org/repo/actions/artifacts/123/zip"), b"zip-data")
        command.assert_called_once_with(["gh", "api", "repos/org/repo/actions/artifacts/123/zip"])


if __name__ == "__main__":
    unittest.main()
