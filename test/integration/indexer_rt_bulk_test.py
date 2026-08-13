#!/usr/bin/env python3
"""Focused integration test for the indexer-assisted RT bulk prototype."""

import argparse
import http.client
import json
import os
from pathlib import Path
import threading
import time

import pymysql


def sql_conn(port):
    return pymysql.connect(host="127.0.0.1", port=port, user="root", autocommit=True)


def query(conn, statement, args=None):
    with conn.cursor() as cur:
        cur.execute(statement, args)
        return cur.fetchall()


def count(conn, table):
    return query(conn, f"SELECT COUNT(*) FROM {table}")[0][0]


def table_status(conn, table):
    return dict(query(conn, f"SHOW TABLE {table} STATUS"))


def staging_dirs(data_dir):
    directories = []
    for root, names, _ in os.walk(data_dir):
        for name in names:
            if name.startswith("indexer-rt-bulk-"):
                path = Path(root, name)
                if path.exists():
                    directories.append(path)
    return directories


def indexer_pids():
    pids = []
    for entry in Path("/proc").glob("[0-9]*"):
        try:
            command = (entry / "cmdline").read_bytes().replace(b"\0", b" ")
        except (FileNotFoundError, PermissionError, ProcessLookupError):
            continue
        if b"indexer_rt_bulk_chunk" in command:
            pids.append(int(entry.name))
    if pids or Path("/proc").exists():
        return pids
    output = os.popen("ps -axo pid=,command=").read().splitlines()
    return [int(line.split(None, 1)[0]) for line in output if "indexer_rt_bulk_chunk" in line]


def wait_for_streaming_indexer(data_dir, timeout=5):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        directories = staging_dirs(data_dir)
        if len(directories) == 1 and indexer_pids():
            assert not (directories[0] / "input.csv").exists(), "streaming path created an input.csv"
            return directories[0]
        time.sleep(0.02)
    raise AssertionError("indexer did not start before request/transaction completion")


def sql_transaction_test(port, data_dir):
    writer = sql_conn(port)
    reader = sql_conn(port)
    query(writer, "DROP TABLE IF EXISTS indexer_rt_sql")
    query(writer, "CREATE TABLE indexer_rt_sql(title TEXT, gid INTEGER, price FLOAT)")
    query(writer, "INSERT INTO indexer_rt_sql(id,title,gid,price) VALUES (10,'existing RT row',100,10.0)")
    query(writer, "SET indexer_rt_bulk=1")
    query(writer, "BEGIN")
    query(writer, "INSERT INTO indexer_rt_sql(id,title,gid,price) VALUES (1,'comma, quote \\\' and line\\nfeed',10,1.25)")
    wait_for_streaming_indexer(data_dir)
    # This exceeds normal socket and pipe buffers. Returning from INSERT before
    # COMMIT proves the indexer/csvpipe consumer is actively draining the stream.
    large_title = "second searchable row " + "x" * (2 * 1024 * 1024)
    query(writer, "INSERT INTO indexer_rt_sql(id,title,gid,price) VALUES (2,%s,20,2.5)", (large_title,))
    assert count(reader, "indexer_rt_sql") == 1, "staged SQL rows became visible before COMMIT"
    query(writer, "COMMIT")
    assert count(reader, "indexer_rt_sql") == 3
    rows = query(reader, "SELECT id,gid,price FROM indexer_rt_sql WHERE MATCH('searchable')")
    assert rows == ((2, 20, 2.5),), rows

    query(writer, "BEGIN")
    query(writer, "INSERT INTO indexer_rt_sql(id,title,gid,price) VALUES (3,'rollback row',30,3.5)")
    query(writer, "ROLLBACK")
    assert count(reader, "indexer_rt_sql") == 3, "ROLLBACK leaked a staged row"
    writer.close()
    reader.close()


def sql_disconnect_cleanup_test(port, data_dir):
    writer = sql_conn(port)
    reader = sql_conn(port)
    query(writer, "DROP TABLE IF EXISTS indexer_rt_disconnect")
    query(writer, "CREATE TABLE indexer_rt_disconnect(title TEXT, gid INTEGER)")
    query(writer, "SET indexer_rt_bulk=1")
    query(writer, "BEGIN")
    query(writer, "INSERT INTO indexer_rt_disconnect(id,title,gid) VALUES (11,'abandoned row',1)")
    assert count(reader, "indexer_rt_disconnect") == 0
    writer.close()
    deadline = time.monotonic() + 5
    while staging_dirs(data_dir) and time.monotonic() < deadline:
        time.sleep(0.02)
    assert not staging_dirs(data_dir), "disconnect left a staging directory behind"
    assert count(reader, "indexer_rt_disconnect") == 0

    writer = sql_conn(port)
    query(writer, "SET indexer_rt_bulk=1")
    try:
        query(writer, "INSERT INTO indexer_rt_disconnect(id,title,gid) VALUES (12,'missing begin',2)")
        raise AssertionError("bulk mode unexpectedly accepted INSERT outside BEGIN")
    except pymysql.MySQLError as exc:
        assert "requires an active transaction" in str(exc), exc
    assert not staging_dirs(data_dir), "INSERT outside BEGIN left staging state behind"

    query(writer, "BEGIN")
    query(writer, "INSERT INTO indexer_rt_disconnect(id,title,gid) VALUES (13,'staged before replace',3)")
    try:
        query(writer, "REPLACE INTO indexer_rt_disconnect(id,title,gid) VALUES (14,'unsupported replace',4)")
        raise AssertionError("bulk mode unexpectedly accepted REPLACE")
    except pymysql.MySQLError as exc:
        assert "supports INSERT only" in str(exc), exc
    deadline = time.monotonic() + 5
    while staging_dirs(data_dir) and time.monotonic() < deadline:
        time.sleep(0.02)
    assert not staging_dirs(data_dir), "rejected REPLACE did not cancel the streaming pipeline"
    query(writer, "COMMIT")
    assert count(reader, "indexer_rt_disconnect") == 0, "COMMIT attached rows from a failed ingestion"
    writer.close()
    reader.close()


def concurrent_streams_test(port, data_dir):
    first = sql_conn(port)
    second = sql_conn(port)
    reader = sql_conn(port)
    query(reader, "DROP TABLE IF EXISTS indexer_rt_concurrent_a")
    query(reader, "DROP TABLE IF EXISTS indexer_rt_concurrent_b")
    query(reader, "CREATE TABLE indexer_rt_concurrent_a(title TEXT)")
    query(reader, "CREATE TABLE indexer_rt_concurrent_b(title TEXT)")
    for connection, table, docid in (
        (first, "indexer_rt_concurrent_a", 1),
        (second, "indexer_rt_concurrent_b", 2),
    ):
        query(connection, "SET indexer_rt_bulk=1")
        query(connection, "BEGIN")
        query(connection, f"INSERT INTO {table}(id,title) VALUES ({docid},'concurrent stream')")

    deadline = time.monotonic() + 5
    while len(indexer_pids()) < 2 and time.monotonic() < deadline:
        time.sleep(0.02)
    assert len(indexer_pids()) >= 2, "two streaming transactions did not start two indexers"

    outcome = {}

    def commit_first():
        outcome["rows"] = query(first, "COMMIT")

    committer = threading.Thread(target=commit_first, daemon=True)
    committer.start()
    committer.join(timeout=10)
    assert not committer.is_alive(), "first COMMIT waited for the second stream to close"
    assert count(reader, "indexer_rt_concurrent_a") == 1
    assert count(reader, "indexer_rt_concurrent_b") == 0
    query(second, "ROLLBACK")
    first.close()
    second.close()
    reader.close()
    assert not staging_dirs(data_dir), "concurrent streams left staging state behind"


def transaction_invariant_test(port, data_dir):
    writer = sql_conn(port)
    admin = sql_conn(port)
    query(admin, "DROP TABLE IF EXISTS indexer_rt_invariant")
    query(admin, "CREATE TABLE indexer_rt_invariant(title TEXT)")
    query(writer, "SET indexer_rt_bulk=1")
    query(writer, "BEGIN")
    query(writer, "INSERT INTO indexer_rt_invariant(id,title) VALUES (1,'cancel on mode change')")
    wait_for_streaming_indexer(data_dir)
    query(writer, "SET autocommit=1")
    assert not staging_dirs(data_dir), "SET autocommit did not cancel the streaming transaction"
    assert count(admin, "indexer_rt_invariant") == 0

    query(writer, "BEGIN")
    query(writer, "INSERT INTO indexer_rt_invariant(id,title) VALUES (2,'old table object')")
    wait_for_streaming_indexer(data_dir)
    query(admin, "DROP TABLE indexer_rt_invariant")
    query(admin, "CREATE TABLE indexer_rt_invariant(title TEXT)")
    try:
        query(writer, "COMMIT")
        raise AssertionError("COMMIT attached the chunk to a replacement table")
    except pymysql.MySQLError as exc:
        assert "was replaced" in str(exc), exc
    assert count(admin, "indexer_rt_invariant") == 0
    assert not staging_dirs(data_dir), "replacement-table rejection left staging state behind"
    writer.close()
    admin.close()


def send_chunked_bulk(port, first_sent, finish, response):
    conn = http.client.HTTPConnection("127.0.0.1", port, timeout=30)
    conn.putrequest("POST", "/bulk?indexer_rt_bulk=1")
    conn.putheader("Content-Type", "application/x-ndjson")
    conn.putheader("Transfer-Encoding", "chunked")
    conn.endheaders()

    def chunk(obj):
        payload = (json.dumps(obj, separators=(",", ":")) + "\n").encode()
        conn.send(f"{len(payload):X}\r\n".encode() + payload + b"\r\n")

    chunk({"insert": {"index": "indexer_rt_http", "id": 101, "doc": {"title": "first streamed row " + "x" * 16384, "gid": 1}}})
    first_sent.set()
    finish.wait(timeout=20)
    chunk({"insert": {"index": "indexer_rt_http", "id": 102, "doc": {"title": "second streamed row", "gid": 2}}})
    conn.send(b"0\r\n\r\n")
    reply = conn.getresponse()
    response["status"] = reply.status
    response["body"] = json.loads(reply.read())
    conn.close()


def http_streaming_test(sql_port, http_port, data_dir):
    reader = sql_conn(sql_port)
    query(reader, "DROP TABLE IF EXISTS indexer_rt_http")
    query(reader, "CREATE TABLE indexer_rt_http(title TEXT, gid INTEGER)")

    first_sent = threading.Event()
    finish = threading.Event()
    response = {}
    sender = threading.Thread(target=send_chunked_bulk, args=(http_port, first_sent, finish, response), daemon=True)
    sender.start()
    assert first_sent.wait(timeout=5), "sender did not transmit the first HTTP chunk"
    wait_for_streaming_indexer(data_dir)
    assert count(reader, "indexer_rt_http") == 0, "HTTP row became visible before request EOF"
    finish.set()
    sender.join(timeout=30)
    assert not sender.is_alive(), "chunked bulk request did not finish"
    assert response["status"] == 200, response
    assert response["body"]["errors"] is False, response
    assert count(reader, "indexer_rt_http") == 2
    assert int(table_status(reader, "indexer_rt_http")["disk_chunks"]) == 1
    assert not staging_dirs(data_dir), "successful HTTP bulk left staging state behind"
    ids = {row[0] for row in query(reader, "SELECT id FROM indexer_rt_http WHERE MATCH('streamed')")}
    assert ids == {101, 102}, ids
    reader.close()


def malformed_http_cleanup_test(sql_port, http_port, data_dir):
    reader = sql_conn(sql_port)
    query(reader, "DROP TABLE IF EXISTS indexer_rt_http_bad")
    query(reader, "CREATE TABLE indexer_rt_http_bad(title TEXT, gid INTEGER)")

    conn = http.client.HTTPConnection("127.0.0.1", http_port, timeout=30)
    body = (
        json.dumps({"insert": {"index": "indexer_rt_http_bad", "id": 201, "doc": {"title": "must rollback", "gid": 1}}})
        + "\n{not-json}\n"
    )
    conn.request("POST", "/bulk?indexer_rt_bulk=1", body=body, headers={"Content-Type": "application/x-ndjson"})
    reply = conn.getresponse()
    reply.read()
    conn.close()
    assert reply.status == 400, reply.status
    assert count(reader, "indexer_rt_http_bad") == 0
    assert not staging_dirs(data_dir), "failed HTTP bulk left staging state behind"
    reader.close()


def unsupported_http_operation_test(sql_port, http_port, data_dir):
    reader = sql_conn(sql_port)
    query(reader, "DROP TABLE IF EXISTS indexer_rt_http_ops")
    query(reader, "CREATE TABLE indexer_rt_http_ops(title TEXT, gid INTEGER)")
    query(reader, "INSERT INTO indexer_rt_http_ops(id,title,gid) VALUES (301,'keep me',1)")

    conn = http.client.HTTPConnection("127.0.0.1", http_port, timeout=30)
    body = json.dumps({"delete": {"index": "indexer_rt_http_ops", "id": 301}}) + "\n"
    conn.request("POST", "/bulk?indexer_rt_bulk=1", body=body, headers={"Content-Type": "application/x-ndjson"})
    reply = conn.getresponse()
    payload = json.loads(reply.read())
    conn.close()
    assert reply.status == 400, (reply.status, payload)
    assert "supports INSERT only" in payload["error"], payload
    assert count(reader, "indexer_rt_http_ops") == 1
    assert not staging_dirs(data_dir), "rejected HTTP operation left staging state behind"
    reader.close()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--sql-port", type=int, default=19306)
    parser.add_argument("--http-port", type=int, default=19308)
    parser.add_argument("--data-dir", default="/tmp/manticore-indexer-rt/data")
    args = parser.parse_args()
    sql_transaction_test(args.sql_port, args.data_dir)
    sql_disconnect_cleanup_test(args.sql_port, args.data_dir)
    concurrent_streams_test(args.sql_port, args.data_dir)
    transaction_invariant_test(args.sql_port, args.data_dir)
    http_streaming_test(args.sql_port, args.http_port, args.data_dir)
    malformed_http_cleanup_test(args.sql_port, args.http_port, args.data_dir)
    unsupported_http_operation_test(args.sql_port, args.http_port, args.data_dir)
    print("indexer-assisted SQL transaction and chunked HTTP bulk tests: PASS")


if __name__ == "__main__":
    main()
