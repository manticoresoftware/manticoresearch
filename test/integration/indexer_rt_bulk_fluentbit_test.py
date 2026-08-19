#!/usr/bin/env python
"""Verify Fluent Bit-compatible Elasticsearch bulk uses indexer-assisted RT insertion."""

from __future__ import print_function

import argparse
try:
    import http.client as http_client
except ImportError:
    import httplib as http_client
import json
import os
import shutil
import signal
import socket
import subprocess
import tempfile
import time


def free_port():
    sock = socket.socket()
    try:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]
    finally:
        sock.close()


def request(port, path, body):
    connection = http_client.HTTPConnection("127.0.0.1", port, timeout=30)
    connection.request("POST", path, body=body, headers={"Content-Type": "application/x-ndjson"})
    response = connection.getresponse()
    payload = response.read().decode("utf-8", errors="replace")
    connection.close()
    return response.status, payload


def sql(port, statement):
    status, payload = request(port, "/sql?mode=raw", statement)
    assert status == 200, (statement, status, payload)
    result = json.loads(payload)[0]
    assert not result.get("error"), (statement, result)
    return result.get("data", [])


def count(port, table):
    return sql(port, "SELECT COUNT(*) FROM {}".format(table))[0]["count(*)"]


def row(port, table, docid):
    rows = sql(port, "SELECT id,title,gid FROM {} WHERE id={}".format(table, docid))
    return rows[0] if rows else None


def disk_chunks(port, table):
    status = sql(port, "SHOW TABLE {} STATUS".format(table))
    return int(dict((item["Variable_name"], item["Value"]) for item in status)["disk_chunks"])


def bulk(port, lines, pipeline=True):
    path = "/_bulk/?pipeline=indexer_rt_bulk" if pipeline else "/_bulk/"
    body = "".join(json.dumps(line, separators=(",", ":")) + "\n" for line in lines)
    status, payload = request(port, path, body)
    return status, json.loads(payload)


def wait_ready(port, process):
    deadline = time.monotonic() + 10 if hasattr(time, "monotonic") else time.time() + 10
    now = time.monotonic if hasattr(time, "monotonic") else time.time
    last_error = ""
    while now() < deadline:
        if process.poll() is not None:
            raise AssertionError("searchd exited during startup with status {}".format(process.returncode))
        try:
            status, payload = request(port, "/sql?mode=raw", "SELECT 1")
            if status == 200 and '"1":1' in payload:
                return
            last_error = "HTTP {}: {}".format(status, payload)
        except (OSError, socket.error) as exc:
            last_error = str(exc)
        time.sleep(0.05)
    raise AssertionError("searchd did not become ready: {}".format(last_error))


def wait_process(process, timeout):
    deadline = time.time() + timeout
    while process.poll() is None and time.time() < deadline:
        time.sleep(0.05)
    return process.poll() is not None


def staging_dirs(data_dir):
    found = []
    for root, names, _ in os.walk(data_dir):
        for name in names:
            if name.startswith("indexer-rt-bulk-"):
                found.append(os.path.join(root, name))
    return found


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--searchd", required=True)
    args = parser.parse_args()

    root = tempfile.mkdtemp(prefix="manticore-indexer-fluentbit-")
    process = None
    output = None
    try:
        data_dir = os.path.join(root, "data")
        os.mkdir(data_dir)
        port = free_port()
        config = os.path.join(root, "manticore.conf")
        with open(config, "w") as config_file:
            config_file.write(
                "searchd {{\n"
                "  listen = 127.0.0.1:{port}:http\n"
                "  log = {log}\n"
                "  query_log = {query_log}\n"
                "  pid_file = {pid_file}\n"
                "  data_dir = {data_dir}\n"
                "  workers = threads\n"
                "  watchdog = 0\n"
                "}}\n".format(
                    port=port,
                    log=os.path.join(root, "searchd.log"),
                    query_log=os.path.join(root, "query.log"),
                    pid_file=os.path.join(root, "searchd.pid"),
                    data_dir=data_dir,
                )
            )

        environment = os.environ.copy()
        environment["MANTICORE_NO_BUDDY"] = "1"
        output = open(os.path.join(root, "searchd-console.log"), "wb")
        process = subprocess.Popen(
            [args.searchd, "--config", config, "--nodetach"],
            cwd=root,
            env=environment,
            stdout=output,
            stderr=output,
        )
        wait_ready(port, process)

        table = "indexer_rt_fluent_bit"
        other = table + "_other"
        sql(port, "CREATE TABLE {}(title TEXT, gid INTEGER)".format(table))
        sql(port, "CREATE TABLE {}(title TEXT, gid INTEGER)".format(other))

        status, payload = bulk(port, [
            {"index": {"_index": table, "_id": "401"}},
            {"title": "first fluent bit row", "gid": 1},
            {"index": {"_index": table, "_id": "402"}},
            {"title": "second fluent bit row", "gid": 2},
        ])
        assert status == 200 and payload["errors"] is False, payload
        assert payload["items"] == [], payload
        assert count(port, table) == 2
        assert disk_chunks(port, table) == 1

        status, payload = bulk(port, [
            {"index": {"_index": table, "_id": "401"}},
            {"title": "replaced fluent bit row", "gid": 10},
        ])
        assert status == 200 and payload["errors"] is False, payload
        assert count(port, table) == 2
        assert row(port, table, 401) == {"id": 401, "title": "replaced fluent bit row", "gid": 10}

        status, payload = bulk(port, [
            {"index": {"_index": table, "_id": "405"}},
            {"title": "must not attach to first", "gid": 5},
            {"index": {"_index": other, "_id": "406"}},
            {"title": "must not attach to second", "gid": 6},
        ])
        assert status == 400 and "one target table" in payload["error"]["reason"], payload
        assert count(port, table) == 2 and count(port, other) == 0

        status, payload = bulk(port, [
            {"index": {"_index": table, "_id": "408"}},
            {"title": "first duplicate", "gid": 8},
            {"index": {"_index": table, "_id": "408"}},
            {"title": "second duplicate", "gid": 9},
        ])
        assert status == 400 and "unique document ids" in payload["error"]["reason"], payload
        assert count(port, table) == 2

        status, payload = bulk(port, [
            {"create": {"_index": table, "_id": "407"}},
            {"title": "unsupported create", "gid": 7},
        ])
        assert status == 400 and "bulk index actions only" in payload["error"]["reason"], payload
        assert count(port, table) == 2

        status, payload = request(port, "/_bulk/?pipeline=indexer_rt_bulk", "\n")
        payload = json.loads(payload)
        assert status == 400 and "at least one document" in payload["error"]["reason"], payload

        before_chunks = disk_chunks(port, table)
        status, payload = bulk(port, [
            {"index": {"_index": table, "_id": "408"}},
            {"title": "must not attach", "gid": 8},
            {"index": {"_index": table}},
            {"title": "missing id", "gid": 9},
        ])
        assert status == 400 and "explicit non-zero numeric _id" in payload["error"]["reason"], payload
        assert count(port, table) == 2 and disk_chunks(port, table) == before_chunks

        status, payload = bulk(port, [
            {"create": {"_index": table, "_id": "403"}},
            {"title": "ordinary trailing slash", "gid": 3},
        ], pipeline=False)
        assert status == 200 and payload["errors"] is False, payload
        assert len(payload["items"]) == 1 and count(port, table) == 3
        assert not staging_dirs(data_dir), staging_dirs(data_dir)
        assert process.poll() is None, "Fluent Bit bulk test stopped searchd"
    finally:
        if process is not None and process.poll() is None:
            process.send_signal(signal.SIGTERM)
            if not wait_process(process, 10):
                process.kill()
                wait_process(process, 5)
        if output is not None:
            output.close()
        shutil.rmtree(root, ignore_errors=True)


if __name__ == "__main__":
    main()
