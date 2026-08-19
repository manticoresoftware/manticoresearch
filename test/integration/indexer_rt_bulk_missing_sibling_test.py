#!/usr/bin/env python
"""Verify that indexer-assisted ingestion lazily requires an absolute sibling."""

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
    connection = http_client.HTTPConnection("127.0.0.1", port, timeout=5)
    connection.request("POST", path, body=body, headers={"Content-Type": "application/x-ndjson"})
    response = connection.getresponse()
    payload = response.read().decode("utf-8", errors="replace")
    connection.close()
    return response.status, payload


def sql(port, statement):
    return request(port, "/sql?mode=raw", statement)


def wait_ready(port, process):
    deadline = time.monotonic() + 10 if hasattr(time, "monotonic") else time.time() + 10
    now = time.monotonic if hasattr(time, "monotonic") else time.time
    last_error = ""
    while now() < deadline:
        if process.poll() is not None:
            raise AssertionError("searchd exited during startup with status {}".format(process.returncode))
        try:
            status, payload = sql(port, "SELECT 1")
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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--searchd", required=True)
    args = parser.parse_args()

    root = tempfile.mkdtemp(prefix="manticore-indexer-sibling-")
    process = None
    devnull = None
    try:
        actual_bindir = os.path.join(root, "actual")
        launch_bindir = os.path.join(root, "launch")
        data_dir = os.path.join(root, "data")
        os.mkdir(actual_bindir)
        os.mkdir(launch_bindir)
        os.mkdir(data_dir)
        searchd = os.path.join(actual_bindir, "searchd")
        launch_searchd = os.path.join(launch_bindir, "searchd")
        shutil.copy2(args.searchd, searchd)
        os.symlink(searchd, launch_searchd)
        missing_indexer = os.path.realpath(os.path.join(actual_bindir, "indexer"))
        path_marker = os.path.join(root, "path-indexer-ran")
        path_indexer = os.path.join(launch_bindir, "indexer")
        with open(path_indexer, "w") as script:
            script.write("#!/bin/sh\n: > '{}'\nexit 0\n".format(path_marker.replace("'", "'\\''")))
        os.chmod(path_indexer, 0o755)
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
        environment["MANTICORE_INDEXER_RT_INDEXER"] = path_indexer
        environment["PATH"] = launch_bindir
        devnull = open(os.devnull, "wb")
        process = subprocess.Popen(
            [launch_searchd, "--config", config, "--nodetach"],
            cwd=root,
            env=environment,
            stdout=devnull,
            stderr=devnull,
        )

        wait_ready(port, process)
        status, payload = sql(port, "CREATE TABLE missing_sibling(title TEXT)")
        assert status == 200, payload

        body = json.dumps({"insert": {"index": "missing_sibling", "id": 1, "doc": {"title": "test"}}}) + "\n"
        status, payload = request(port, "/bulk?indexer_rt_bulk=1", body)
        assert status != 200, payload
        assert missing_indexer in payload, payload
        assert "indexer" in payload.lower(), payload
        assert not os.path.exists(path_marker), "searchd launched indexer from PATH"

        diagnostic = "diagnostic-marker-from-indexer"
        with open(missing_indexer, "w") as script:
            script.write("#!/bin/sh\n/bin/cat >/dev/null\necho '{}' >&2\nexit 1\n".format(diagnostic))
        os.chmod(missing_indexer, 0o755)
        status, payload = request(port, "/bulk?indexer_rt_bulk=1", body)
        assert status != 200, payload
        assert diagnostic in payload, payload

        status, payload = sql(port, "SELECT COUNT(*) FROM missing_sibling")
        assert status == 200 and '"count(*)":0' in payload.lower(), payload
        assert process.poll() is None, "missing sibling stopped searchd"
    finally:
        if process is not None and process.poll() is None:
            process.send_signal(signal.SIGTERM)
            if not wait_process(process, 10):
                process.kill()
                wait_process(process, 5)
        if devnull is not None:
            devnull.close()
        shutil.rmtree(root, ignore_errors=True)


if __name__ == "__main__":
    main()
