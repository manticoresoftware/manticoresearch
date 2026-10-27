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
    def __init__(self, binary, root, budget, threads=None):
        self.binary, self.root = binary, root
        self.port = free_port()
        (root / "data").mkdir(exist_ok=True)
        self.config = root / "manticore.conf"
        threads_config = f"threads = {threads}\n" if threads is not None else ""
        self.config.write_text(
            "searchd {\n"
            f"listen = 127.0.0.1:{self.port}:http\n"
            f"data_dir = {root / 'data'}\n"
            f"log = {root / 'searchd.log'}\n"
            f"pid_file = {root / 'searchd.pid'}\n"
            "buddy_path =\n"
            f"docid_lookup_cache_size = {budget}\n"
            f"{threads_config}"
            "}\n"
        )

    def sql(self, query):
        return self.sql_all(query)[0]

    def sql_all(self, query):
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
        return results

    def used(self):
        return int(self.sql("SHOW STATUS LIKE 'docid_lookup_cache_used_bytes'")["data"][0]["Value"])

    @contextlib.contextmanager
    def running(self):
        with (self.root / "console.log").open("a") as output:
            proc = subprocess.Popen(
                [self.binary, "--config", str(self.config), "--nodetach"],
                stdout=output, stderr=subprocess.STDOUT,
            )
            self.pid = proc.pid
            try:
                deadline = time.monotonic() + 30
                while True:
                    if proc.poll() is not None:
                        raise RuntimeError(f"searchd exited with status {proc.returncode}:\n" + (self.root / "console.log").read_text())
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
                    raise RuntimeError(f"searchd exited with status {proc.returncode}:\n" + (self.root / "console.log").read_text())


def check(binary, budget, expect_cache):
    with tempfile.TemporaryDirectory(prefix="manticore-native-docid-") as temp:
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
            # Automatic planning must see cache admission, including when a
            # budget refuses the chunk. Explicit hints remain authoritative.
            candidates = [i * 154 + 2 for i in range(129)]
            clause = ",".join(map(str, candidates + [3, candidates[15]]))
            for suffix in ("", " /*+ DocidIndex(id) */", " /*+ NO_DocidIndex(id) */"):
                result, meta = db.sql_all(
                    f"SELECT count(*) AS n FROM docs WHERE id IN ({clause}){suffix}; SHOW META"
                )
                assert result["data"] == [{"n": 129}], result
                plan = [row["Value"] for row in meta["data"] if row["Variable_name"] == "index"]
                if "NO_DocidIndex" in suffix:
                    assert not any("DocidIndex" in value for value in plan), plan
                elif expect_cache or suffix:
                    assert any("DocidIndex" in value for value in plan), plan
            for predicate in (f"MATCH('document') AND id IN ({clause})",
                              f"id IN ({clause}) OR value=987654"):
                assert db.sql(f"SELECT count(*) AS n FROM docs WHERE {predicate}")["data"] == [{"n": 129}]
            assert db.sql("SELECT id FROM docs WHERE id=2")["data"] == [{"id": 2}]
            assert not db.sql("SELECT id FROM docs WHERE id=3")["data"]
            db.sql("UPDATE docs SET value=17 WHERE id IN (2,100,20000,3)")
            assert db.sql("SELECT id,value FROM docs WHERE id IN (2,100,20000) ORDER BY id ASC")["data"] == [
                {"id": i, "value": 17} for i in (2, 100, 20000)
            ]
            db.sql("DELETE FROM docs WHERE id IN (4,6,99999)")
            db.sql("UPDATE docs SET value=31 WHERE id IN (4,6,8,8,99999)")
            assert not db.sql("SELECT id FROM docs WHERE id IN (4,6)")["data"]
            assert db.sql("SELECT value FROM docs WHERE id=8")["data"] == [{"value": 31}]
            db.sql("UPDATE docs SET value=37 WHERE id IN (100,20000) AND value=17")
            assert db.sql("SELECT value FROM docs WHERE id=100")["data"] == [{"value": 37}]
            # Cross full tiles and the scalar tail with repeated, dead and
            # missing IDs. A second predicate also exercises SELECT filtering.
            for count in (15, 16, 17, 31, 32, 33, 129):
                existing = [i * 2 + 2 for i in range(count) if i * 2 + 2 not in (4, 6)]
                candidates = existing + [4, 6, 3, 99999] + existing[14:18]
                clause = ",".join(map(str, candidates))
                value = 100 + count
                db.sql(f"UPDATE docs SET value={value} WHERE id IN ({clause})")
                assert db.sql(
                    f"SELECT count(*) AS n FROM docs WHERE id IN ({clause}) AND value={value}"
                )["data"] == [{"n": len(existing)}]
            db.sql("REPLACE INTO docs (id,value,title) VALUES (2,23,'replacement')")
            db.sql("FLUSH RAMCHUNK docs")
            assert not db.sql("SELECT id FROM docs WHERE id IN (4,6)")["data"]
            assert db.sql("SELECT value FROM docs WHERE id=2")["data"] == [{"value": 23}]
            db.sql("ALTER TABLE docs ADD COLUMN added int")
            db.sql("UPDATE docs SET added=9 WHERE id=100")
            assert db.sql("SELECT added FROM docs WHERE id=100")["data"] == [{"added": 9}]
        with server.running() as db:
            # A small chunk created by REPLACE may fit even when the original
            # large chunk did not. Admission is per chunk under one budget.
            assert db.used() <= budget
            if expect_cache:
                assert db.used() > 0
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
    args = parser.parse_args()
    for limit, enabled in [(0, False), (1024, False), (16 * 1024 * 1024, True)]:
        check(args.searchd, limit, enabled)
