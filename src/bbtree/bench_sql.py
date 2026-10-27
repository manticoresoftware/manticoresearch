#!/usr/bin/env python3
"""Compare HTTP/SQL disk UPDATE latency with the optional point cache off/on."""
import argparse
import json
from pathlib import Path
import random
import statistics
import tempfile
import time

from test_integration import Server


def run(binary, ids, queries, rounds, budget):
    with tempfile.TemporaryDirectory(prefix="manticore-bbtree-sql-bench-") as temp:
        server = Server(binary, Path(temp), budget)
        with server.running() as db:
            db.sql("CREATE TABLE docs (value int) engine='rowwise'")
            for start in range(0, len(ids), 5000):
                rows = ",".join(f"({i},0)" for i in ids[start:start + 5000])
                db.sql("INSERT INTO docs (id,value) VALUES " + rows)
            start = time.perf_counter()
            db.sql("FLUSH RAMCHUNK docs")
            flush_ms = (time.perf_counter() - start) * 1000
            used = db.used()
            assert (used > 0) == bool(budget)
            samples = []
            for repeat in range(rounds + 1):
                round_queries = [query.format(value=repeat + 1) for query in queries]
                start = time.perf_counter()
                for query in round_queries:
                    db.sql(query)
                elapsed = (time.perf_counter() - start) * 1000 / len(queries)
                if repeat:  # first round warms the entire SQL path
                    samples.append(elapsed)
            probe = int(queries[-1].split("IN (")[1].split(",")[0])
            assert db.sql(f"SELECT value FROM docs WHERE id={probe}")["data"] == [{"value": rounds + 1}]
            return {
                "budget_bytes": budget, "cache_bytes": used,
                "flush_ms": flush_ms, "update_ms_samples": samples,
                "update_ms_median": statistics.median(samples),
            }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("searchd", type=lambda value: str(Path(value).resolve()))
    parser.add_argument("--docs", type=int, default=1000000)
    parser.add_argument("--rounds", type=int, default=7)
    parser.add_argument("--queries", type=int, default=50)
    parser.add_argument("--batch", type=int, default=1000)
    args = parser.parse_args()
    rng = random.Random(42)
    ids = rng.sample(range(2, 1 << 62, 2), args.docs)
    queries = [
        "UPDATE docs SET value={value} WHERE id IN (" + ",".join(map(str, rng.sample(ids, args.batch))) + ")"
        for _ in range(args.queries)
    ]
    results = [run(args.searchd, ids, queries, args.rounds, budget) for budget in (0, 256 * 1024 * 1024)]
    print(json.dumps({
        "docs": args.docs, "batch": args.batch, "queries_per_round": args.queries,
        "results": results,
        "speedup": results[0]["update_ms_median"] / results[1]["update_ms_median"],
    }, indent=2))
