"""CLT retry and reliability accounting tests."""

import importlib.util
import io
import json
import unittest
import zipfile
from collections import Counter, defaultdict
from pathlib import Path

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


class ReliabilityTest(unittest.TestCase):
    def test_retries_count_only_executed_shards(self):
        run = {"id": 42, "created_at": "2026-10-22T12:00:00Z"}
        data = archive([
            (1, 1, [{"test": "a.rec", "status": "passed"}]),
            (2, 1, [{"test": "b.rec", "status": "failed"}]),
            (2, 2, [{"test": "b.rec", "status": "passed"}]),
        ])
        stats, failures = defaultdict(Counter), {}
        clt.count_results(stats, failures, run, data, "org/repo")
        rows = {row["test"]: row for row in clt.make_rows(stats, failures)}
        self.assertEqual((rows["a.rec"]["passed"], rows["a.rec"]["executions"]), (1, 1))
        self.assertEqual((rows["b.rec"]["passed"], rows["b.rec"]["failed"]), (1, 1))
        self.assertEqual(rows["b.rec"]["failure_rate"], 50)
        self.assertTrue(rows["b.rec"]["last_failure_url"].endswith("/attempts/1"))

    def test_interrupted_and_missing_are_not_successes(self):
        data = archive([(1, 1, [
            {"test": "passed.rec", "status": "passed"},
            {"test": "interrupted.rec", "status": "interrupted"},
            {"test": "missing.rec", "status": "missing"},
        ])])
        stats, failures = defaultdict(Counter), {}
        clt.count_results(stats, failures, {"id": 1, "created_at": "2026-10-22T00:00:00Z"}, data, "org/repo")
        self.assertEqual([row["test"] for row in clt.make_rows(stats, failures)], ["passed.rec"])

    def test_most_recent_failure_uses_latest_attempt(self):
        run = {"id": 42, "created_at": "2026-10-22T12:00:00Z"}
        data = archive([
            (1, 1, [{"test": "a.rec", "status": "failed"}]),
            (1, 2, [{"test": "a.rec", "status": "failed"}]),
        ])
        stats, failures = defaultdict(Counter), {}
        clt.count_results(stats, failures, run, data, "org/repo")
        self.assertTrue(clt.make_rows(stats, failures)[0]["last_failure_url"].endswith("/attempts/2"))
        self.assertEqual(clt.make_rows(stats, failures)[0]["executions"], 2)


    def test_reject_broken_bundle(self):
        with self.assertRaisesRegex(ValueError, "No CLT shard results"):
            list(clt.shard_results(archive([])))

    def test_empty_summary(self):
        self.assertIn("No failing tests", clt.summary([], 2, 0))


if __name__ == "__main__":
    unittest.main()
