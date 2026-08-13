#!/usr/bin/env python3
"""Benchmark dev#2761's indexer-assisted HTTP bulk path against direct RT bulk.

Generate an NDJSON corpus with rows shaped as Manticore /bulk insert operations,
then start searchd and run this script under a one-CPU limiter, for example:

    ROWS=1000000 REPS=3 DATA=/path/to/taxi.ndjson \
      cpulimit --include-children -l 100 python3 indexer_rt_bulk_benchmark.py

The daemon must listen on MYSQL_PORT/HTTP_PORT and have
MANTICORE_INDEXER_RT_INDEXER pointing to the matching indexer binary.
"""
import http.client
import json
import os
from pathlib import Path
import statistics
import threading
import time

import pymysql

ROWS = int(os.environ.get("ROWS", "1000000"))
REPS = int(os.environ.get("REPS", "3"))
BATCH = int(os.environ.get("BATCH", "1000"))
HOST = os.environ.get("HOST", "127.0.0.1")
MYSQL_PORT = int(os.environ.get("MYSQL_PORT", "19406"))
HTTP_PORT = int(os.environ.get("HTTP_PORT", "19408"))
DATA = Path(os.environ["DATA"])
SCHEMA = "(pickup_datetime timestamp, dropoff_datetime timestamp, passenger_count int, trip_distance float, fare_amount float, pickup text, dropoff text)"


def sql(statement):
    conn = pymysql.connect(host=HOST, port=MYSQL_PORT, user="root", autocommit=True)
    try:
        with conn.cursor() as cur:
            cur.execute(statement)
            return cur.fetchall()
    finally:
        conn.close()


def prepare(table):
    sql(f"DROP TABLE IF EXISTS {table}")
    sql(f"CREATE TABLE {table} {SCHEMA}")


def request(path, body, encode_chunked=False):
    conn = http.client.HTTPConnection(HOST, HTTP_PORT, timeout=600)
    conn.request("POST", path, body=body, headers={"Content-Type": "application/x-ndjson"}, encode_chunked=encode_chunked)
    reply = conn.getresponse()
    payload = reply.read()
    conn.close()
    if reply.status != 200:
        raise RuntimeError(f"HTTP {reply.status}: {payload[:1000]!r}")
    parsed = json.loads(payload)
    if parsed.get("errors"):
        raise RuntimeError(str(parsed)[:2000])


def direct_rt(table):
    with DATA.open("rb") as source:
        batch = []
        for line in source:
            batch.append(line.replace(b'"table":"taxi"', f'"table":"{table}"'.encode(), 1))
            if len(batch) == BATCH:
                request("/bulk", b"".join(batch))
                batch.clear()
        if batch:
            request("/bulk", b"".join(batch))


def chunks(table):
    old = b'"table":"taxi"'
    new = f'"table":"{table}"'.encode()
    with DATA.open("rb") as source:
        block = bytearray()
        for line in source:
            block.extend(line.replace(old, new, 1))
            if len(block) >= 1024 * 1024:
                yield bytes(block)
                block.clear()
        if block:
            yield bytes(block)


def indexer_rt(table):
    request("/bulk?indexer_rt_bulk=1", chunks(table), encode_chunked=True)


def descendants_rss(root_pid):
    try:
        rows = os.popen("ps -axo pid=,ppid=,rss=").read().splitlines()
        procs = [tuple(map(int, row.split())) for row in rows]
    except Exception:
        return 0
    active = {root_pid}
    changed = True
    while changed:
        changed = False
        for pid, ppid, _ in procs:
            if ppid in active and pid not in active:
                active.add(pid)
                changed = True
    return sum(rss for pid, _, rss in procs if pid in active)


def run(mode, rep):
    table = f"taxi_{mode}_{rep}"
    prepare(table)
    root_pid = os.getppid()
    peak = 0
    stop = threading.Event()

    def sample():
        nonlocal peak
        while not stop.is_set():
            peak = max(peak, descendants_rss(root_pid))
            stop.wait(0.05)

    sampler = threading.Thread(target=sample, daemon=True)
    sampler.start()
    started = time.perf_counter()
    (direct_rt if mode == "direct" else indexer_rt)(table)
    elapsed = time.perf_counter() - started
    stop.set()
    sampler.join()
    count = sql(f"SELECT COUNT(*) FROM {table}")[0][0]
    status = dict(sql(f"SHOW TABLE {table} STATUS"))
    if count != ROWS:
        raise RuntimeError(f"{mode}: expected {ROWS}, got {count}")
    result = {
        "mode": mode,
        "rep": rep,
        "seconds": elapsed,
        "rows_per_sec": ROWS / elapsed,
        "rows": count,
        "disk_chunks": int(status.get("disk_chunks", 0)),
        "ram_bytes": int(status.get("ram_bytes", 0)),
        "disk_bytes": int(status.get("disk_bytes", 0)),
        "peak_tree_rss_kb": peak,
    }
    print(json.dumps(result), flush=True)
    sql(f"DROP TABLE {table}")
    return result


def main():
    results = []
    for rep in range(1, REPS + 1):
        order = ("direct", "indexer") if rep % 2 else ("indexer", "direct")
        for mode in order:
            results.append(run(mode, rep))
    summary = {}
    for mode in ("direct", "indexer"):
        subset = [row for row in results if row["mode"] == mode]
        summary[mode] = {
            "median_seconds": statistics.median(row["seconds"] for row in subset),
            "median_rows_per_sec": statistics.median(row["rows_per_sec"] for row in subset),
            "max_peak_tree_rss_kb": max(row["peak_tree_rss_kb"] for row in subset),
        }
    summary["speedup"] = summary["direct"]["median_seconds"] / summary["indexer"]["median_seconds"]
    print(json.dumps({"summary": summary}, indent=2), flush=True)


if __name__ == "__main__":
    main()
