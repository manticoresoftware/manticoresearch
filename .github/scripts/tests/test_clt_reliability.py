"""Checks for CLT failure-rate accounting and report rendering."""

import importlib.util
import io
import json
import unittest
import zipfile
from collections import Counter, defaultdict
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
        self.stats, self.failures = defaultdict(Counter), {}
        self.run = {"id": 42, "created_at": "2026-10-22T12:00:00Z"}

    def collect(self, shards):
        clt.count_results(self.stats, self.failures, self.run, archive(shards), "org/repo")
        return {r["test"]: r for r in clt.make_rows(self.stats, self.failures)}

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
        self.assertIn("No CLT results available", clt.summary([], 2, 0))
        self.assertIn("No failing tests", clt.summary([], 2, 1))

    @patch.object(clt.subprocess, "check_output", return_value=b"zip-data")
    def test_github_cli_gets_raw_bytes(self, command):
        self.assertEqual(clt.api("repos/org/repo/actions/artifacts/123/zip"), b"zip-data")
        command.assert_called_once_with(["gh", "api", "repos/org/repo/actions/artifacts/123/zip"])


if __name__ == "__main__":
    unittest.main()
