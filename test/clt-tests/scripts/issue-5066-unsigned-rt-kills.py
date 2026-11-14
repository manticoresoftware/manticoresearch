#!/usr/bin/env python3
"""Regression for #5066: background RAM saves must apply mixed-unsigned-ID kills."""

import http.client
import json
import random
import subprocess


IDS = list(range(1, 1001)) + [(1 << 63) + i for i in range(1, 1001)]
ID_MASK = (1 << 64) - 1
TRIALS = 4
ROUNDS = 10
BATCH_SIZE = 100


def sql(query):
    return subprocess.check_output(
        ["mysql", "--protocol=tcp", "-h127.0.0.1", "-P9306", "--batch", "--raw",
         "--skip-column-names", "-e", query],
        text=True,
        timeout=120,
    ).strip()


def replace_batch(conn, table, ids, revision):
    rows = (
        {"replace": {"index": table, "id": doc_id,
                     "doc": {"title": f"revision {revision} id {doc_id} " + "x" * 96,
                             "revision": revision}}}
        for doc_id in ids
    )
    payload = "\n".join(json.dumps(row, separators=(",", ":")) for row in rows) + "\n"
    conn.request("POST", "/bulk", body=payload,
                 headers={"Content-Type": "application/x-ndjson"})
    response = conn.getresponse()
    body = response.read()
    result = json.loads(body)
    # /bulk groups consecutive REPLACEs into one transaction summary, not one item per row.
    items = result.get("items", [])
    bulk = items[0].get("bulk", {}) if len(items) == 1 else {}
    if (response.status != 200 or result.get("errors") is not False
            or result.get("skipped_lines") != 0 or bulk.get("created") != len(ids)):
        raise AssertionError(f"bulk replacement failed: HTTP {response.status}: {body[:500]!r}")


def raw_sql_rows(query):
    # /sql?mode=raw returns named JSON columns, independently of mysql CLI formatting.
    conn = http.client.HTTPConnection("127.0.0.1", 9308, timeout=120)
    try:
        conn.request("POST", "/sql?mode=raw", body=query,
                     headers={"Content-Type": "application/x-www-form-urlencoded"})
        response = conn.getresponse()
        body = response.read()
        try:
            result = json.loads(body)
        except ValueError as exc:
            raise AssertionError(f"invalid SQL response: HTTP {response.status}: {body[:500]!r}") from exc
        if (response.status != 200 or not isinstance(result, list)
                or len(result) != 1 or result[0].get("error")):
            raise AssertionError(f"SQL query failed: HTTP {response.status}: {body[:500]!r}")
        rows = result[0].get("data")
        if not isinstance(rows, list):
            raise AssertionError(f"SQL query has no result rows: {body[:500]!r}")
        return rows
    finally:
        conn.close()


def verify(table):
    # Scan all rows, not WHERE id=... (which may take a document-ID lookup shortcut).
    rows = raw_sql_rows(f"SELECT id, revision FROM {table} LIMIT 10000 OPTION max_matches=10000")
    revisions = {}
    for row in rows:
        if not isinstance(row, dict) or "id" not in row or "revision" not in row:
            raise AssertionError(f"unexpected SQL result row: {row!r}")
        doc_id = int(row["id"]) & ID_MASK
        revision = int(row["revision"])
        revisions.setdefault(doc_id, []).append(revision)

    if set(revisions) != set(IDS) or len(rows) != len(IDS):
        duplicates = {doc_id: found for doc_id, found in revisions.items() if len(found) != 1}
        raise AssertionError(f"unexpected row count or IDs: rows={len(rows)}, "
                             f"unique={len(revisions)}, duplicates={list(duplicates.items())[:5]}")
    stale = [(doc_id, found) for doc_id, found in revisions.items() if found != [ROUNDS]]
    if stale:
        raise AssertionError(f"old revisions survived REPLACE: {stale[:5]}")


def main():
    for trial in range(TRIALS):
        table = f"issue5066_{trial}"
        sql(f"DROP TABLE IF EXISTS {table}")
        try:
            sql(f"CREATE TABLE {table} (id bigint, title text, revision integer) "
                "rt_mem_limit='128k' optimize_cutoff='1000000'")
            rng = random.Random(5066 + trial)
            conn = http.client.HTTPConnection("127.0.0.1", 9308, timeout=120)
            try:
                for revision in range(ROUNDS + 1):
                    shuffled = IDS.copy()
                    rng.shuffle(shuffled)
                    for offset in range(0, len(shuffled), BATCH_SIZE):
                        replace_batch(conn, table, shuffled[offset:offset + BATCH_SIZE], revision)
            finally:
                conn.close()

            # Wait for in-flight background saves and publish any remaining RAM segments.
            sql(f"FLUSH RAMCHUNK {table}")
            verify(table)
        finally:
            sql(f"DROP TABLE IF EXISTS {table}")
    print("RT_UNSIGNED_KILLS_OK")


if __name__ == "__main__":
    main()
