"""Pure-Python tests for CLT result collection and reliability counts."""
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parents[1] / "clt_reliability.py"
spec = importlib.util.spec_from_file_location("clt_reliability", SCRIPT)
clt = importlib.util.module_from_spec(spec)
spec.loader.exec_module(clt)


class CLTReportsTest(unittest.TestCase):
    def write_shard(self, root, shard, attempt, tests, results):
        location = root / f"clt-test-results-{shard}"
        location.mkdir(parents=True)
        (location / "manifest.json").write_text(json.dumps({
            "shard": str(shard), "attempt": attempt, "tests": tests,
        }))
        (location / "results.jsonl").write_text(
            "".join(json.dumps(result) + "\n" for result in results)
        )

    def test_merge_counts_scheduled_but_not_run_separately(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.write_shard(root, 1, 1, ["a", "b", "c", "d"], [
                {"test": "a.rec", "status": "passed", "exit_code": 0, "duration_ms": 91},
                {"test": "b.rec", "status": "failed", "exit_code": 1, "duration_ms": 9},
                {"test": "c.rec", "status": "interrupted", "exit_code": None, "duration_ms": 700},
            ])
            report = clt.merge_shards(root, {"repository": "org/repo", "run_id": 4})
            statuses = {r["test"]: r["status"] for r in report["tests"]}
            self.assertEqual(statuses, {
                "a.rec": "passed", "b.rec": "failed", "c.rec": "interrupted", "d.rec": "not_run",
            })
            self.assertEqual(report["tests"][0]["source_attempt"], 1)
            self.assertIn("1 passed", clt.run_summary(report))
            self.assertIn("1 not run", clt.run_summary(report))

    def test_missing_worker_shard_is_not_mistaken_for_success(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.write_shard(root, 1, 1, ["a"], [
                {"test": "a.rec", "status": "passed", "exit_code": 0, "duration_ms": 9},
            ])
            report = clt.merge_shards(root, {"attempt": 2}, [
                {"id": 1, "tests": ["a"]},
                {"id": 2, "tests": ["b", "c"]},
            ])
            self.assertEqual([r["status"] for r in report["tests"]], ["passed", "unknown", "unknown"])
            self.assertEqual(report["shards"][1]["reported"], 0)

    def test_retries_only_count_tests_that_actually_reran(self):
        base = {"schema_version": 1, "repository": "org/repo", "run_id": 100,
                "event": "push", "ref": "refs/heads/main", "created_at": "2026-10-14T12:00:00+00:00"}
        original = {**base, "attempt": 1, "tests": [
            {"test": "a.rec", "shard": "1", "source_attempt": 1, "status": "passed"},
            {"test": "b.rec", "shard": "2", "source_attempt": 1, "status": "failed"},
        ]}
        retried = {**base, "attempt": 2, "created_at": "2026-10-14T12:30:00+00:00", "tests": [
            original["tests"][0],
            {"test": "b.rec", "shard": "2", "source_attempt": 2, "status": "passed"},
        ]}
        rows = {r["test"]: r for r in clt.compute_statistics([retried, original], "org/repo")}
        self.assertEqual(rows["a.rec"]["passed"], 1)
        self.assertEqual(rows["b.rec"]["passed"], 1)
        self.assertEqual(rows["b.rec"]["failed"], 1)
        self.assertEqual(rows["b.rec"]["failure_rate"], 50.0)
        self.assertTrue(rows["b.rec"]["flaky"])
        self.assertEqual(rows["b.rec"]["last_failure_url"],
                         "https://github.com/org/repo/actions/runs/100/attempts/1")

    def test_interrupted_and_not_run_not_false_successes(self):
        report = {"run_id": 55, "attempt": 1, "created_at": "2026-10-14T12:00:00+00:00", "tests": [
            {"test": "a.rec", "shard": "1", "source_attempt": 1, "status": "interrupted"},
            {"test": "b.rec", "shard": "1", "source_attempt": 1, "status": "not_run"},
            {"test": "c.rec", "shard": "1", "source_attempt": 1, "status": "missing"},
        ]}
        rows = {r["test"]: r for r in clt.compute_statistics([report], "org/repo")}
        self.assertEqual(rows["a.rec"]["executed"], 0)
        self.assertEqual(rows["b.rec"]["not_run"], 1)
        self.assertEqual(rows["c.rec"]["missing"], 1)
        self.assertIsNone(rows["a.rec"]["failure_rate"])

    def test_reject_duplicate_outcomes(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.write_shard(root, 1, 1, ["a"], [
                {"test": "a.rec", "status": "passed"},
                {"test": "a.rec", "status": "failed"},
            ])
            with self.assertRaisesRegex(ValueError, "Duplicate outcome"):
                clt.merge_shards(root, {})

    def test_empty_report(self):
        output = clt.failure_summary([], 30, 1, 0, [])
        self.assertIn("No per-test data yet", output)
        self.assertIn("30 days", output)


if __name__ == "__main__":
    unittest.main()
