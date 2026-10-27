#!/usr/bin/env python3
"""Isolated HTTP/SQL lifecycle checks; uses only Python's standard library."""
import argparse
import contextlib
import json
from pathlib import Path
import socket
import subprocess
import tempfile
import time
import urllib.parse
import urllib.request
import urllib.error


def free_port():
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


class Server:
    def __init__(self, binary, root, budget):
        self.binary, self.root = binary, root
        self.port = free_port()
        (root / "data").mkdir(exist_ok=True)
        self.config = root / "manticore.conf"
        self.config.write_text(
            "searchd {\n"
            f"listen = 127.0.0.1:{self.port}:http\n"
            f"data_dir = {root / 'data'}\n"
            f"log = {root / 'searchd.log'}\n"
            f"pid_file = {root / 'searchd.pid'}\n"
            "buddy_path =\n"
            f"docid_lookup_cache_size = {budget}\n"
            "}\n"
        )

    def sql(self, query):
        body = urllib.parse.urlencode({"query": query}).encode()
        try:
            with urllib.request.urlopen(
                f"http://127.0.0.1:{self.port}/sql?mode=raw", body, timeout=30
            ) as response:
                results = json.load(response)
        except urllib.error.HTTPError as error:
            raise RuntimeError(f"{query[:150]}: {error.read().decode()}") from error
        for result in results:
            if result.get("error"):
                raise RuntimeError(f"{query[:150]}: {result['error']}")
        return results[0]

    def used(self):
        return int(self.sql("SHOW STATUS LIKE 'docid_lookup_cache_used_bytes'")["data"][0]["Value"])

    @contextlib.contextmanager
    def running(self):
        with (self.root / "console.log").open("a") as output:
            proc = subprocess.Popen(
                [self.binary, "--config", str(self.config), "--nodetach"],
                stdout=output, stderr=subprocess.STDOUT,
            )
            try:
                deadline = time.monotonic() + 30
                while True:
                    if proc.poll() is not None:
                        raise RuntimeError((self.root / "console.log").read_text())
                    try:
                        self.sql("SHOW STATUS")
                        break
                    except OSError:
                        if time.monotonic() > deadline:
                            raise
                        time.sleep(0.05)
                yield self
            finally:
                if proc.poll() is None:
                    proc.terminate()
                    try:
                        proc.wait(timeout=20)
                    except subprocess.TimeoutExpired:
                        proc.kill()
                        proc.wait()
                if proc.returncode:
                    raise RuntimeError((self.root / "console.log").read_text())


def check(binary, budget, expect_cache):
    with tempfile.TemporaryDirectory(prefix="manticore-bbtree-") as temp:
        root = Path(temp)
        server = Server(binary, root, budget)
        with server.running() as db:
            assert db.used() == 0
            db.sql("CREATE TABLE docs (value int, title text) engine='rowwise'")
            for start in range(0, 10000, 1000):
                rows = ",".join(f"({i * 2 + 2},0,'document')" for i in range(start, start + 1000))
                db.sql("INSERT INTO docs (id,value,title) VALUES " + rows)
            db.sql("FLUSH RAMCHUNK docs")
            used = db.used()
            assert (used > 0) == expect_cache, (budget, used)
            assert used <= budget
            assert db.sql("SELECT id FROM docs WHERE id=2")["data"] == [{"id": 2}]
            assert not db.sql("SELECT id FROM docs WHERE id=3")["data"]
            db.sql("UPDATE docs SET value=17 WHERE id IN (2,100,20000,3)")
            assert db.sql("SELECT id,value FROM docs WHERE id IN (2,100,20000) ORDER BY id ASC")["data"] == [
                {"id": i, "value": 17} for i in (2, 100, 20000)
            ]
            db.sql("DELETE FROM docs WHERE id IN (4,6,99999)")
            db.sql("REPLACE INTO docs (id,value,title) VALUES (2,23,'replacement')")
            db.sql("FLUSH RAMCHUNK docs")
            assert not db.sql("SELECT id FROM docs WHERE id IN (4,6)")["data"]
            assert db.sql("SELECT value FROM docs WHERE id=2")["data"] == [{"value": 23}]
            db.sql("ALTER TABLE docs ADD COLUMN added int")
            db.sql("UPDATE docs SET added=9 WHERE id=100")
            assert db.sql("SELECT added FROM docs WHERE id=100")["data"] == [{"added": 9}]
        with server.running() as db:
            assert (db.used() > 0) == expect_cache
            assert db.sql("SELECT value FROM docs WHERE id=2")["data"] == [{"value": 23}]
            assert not db.sql("SELECT id FROM docs WHERE id IN (4,6)")["data"]
            assert db.sql("SELECT count(*) AS n FROM docs")["data"] == [{"n": 9998}]
            db.sql("DROP TABLE docs")
            deadline = time.monotonic() + 10
            while db.used() and time.monotonic() < deadline:
                time.sleep(0.05)
            assert db.used() == 0
        print(f"PASS budget={budget}: flush/update/delete/replace/alter/restart/drop (initial cache bytes={used})")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("searchd", type=lambda value: str(Path(value).resolve()))
    parser.add_argument("--without-backend", action="store_true")
    args = parser.parse_args()
    cases = [(0, False)] if args.without_backend else [(0, False), (1024, False), (16 * 1024 * 1024, True)]
    for limit, enabled in cases:
        check(args.searchd, limit, enabled)
    if args.without_backend:
        with tempfile.TemporaryDirectory(prefix="manticore-bbtree-unsupported-") as temp:
            try:
                with Server(args.searchd, Path(temp), 1024).running():
                    raise AssertionError("unsupported nonzero budget accepted")
            except RuntimeError as error:
                assert "requires a build with WITH_BBTREE_DOCID=ON" in str(error), error
        print("PASS nonzero budget rejected when backend is absent")
