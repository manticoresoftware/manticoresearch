#!/usr/bin/env python3
"""Check that the docstore enable switch selects adaptive caching or cache off."""
import argparse
from pathlib import Path
import tempfile

from test_integration import Server


def check(binary, name, settings, enabled):
    with tempfile.TemporaryDirectory(prefix="manticore-docstore-config-") as temp:
        server = Server(binary, Path(temp), 0, threads=2)
        server.config.write_text(server.config.read_text().replace("buddy_path =\n", "buddy_path =\n" + settings))
        with server.running() as db:
            db.sql("CREATE TABLE docs (body text stored) engine='rowwise' "
                   "docstore_block_size='16384' docstore_compression='lz4'")
            body = "stored payload " * 8
            values = ",".join(f"({i},'{body}')" for i in range(1, 1025))
            db.sql("INSERT INTO docs (id,body) VALUES " + values)
            db.sql("FLUSH RAMCHUNK docs")
            for _ in range(3):
                assert db.sql("SELECT body FROM docs WHERE id=1")["data"] == [{"body": body}]
            stats = {r["Counter"]: int(r["Value"]) for r in db.sql("SHOW STATUS LIKE 'docstore_cache_%'")["data"]}
            assert (stats["docstore_cache_used_bytes"] > 0) == enabled, (name, stats)
            assert (stats["docstore_cache_mapped_bytes"] > 0) == enabled, (name, stats)
            db.sql("REPLACE INTO docs (id,body) VALUES (1,'replacement')")
            db.sql("FLUSH RAMCHUNK docs")
            assert db.sql("SELECT body FROM docs WHERE id=1")["data"] == [{"body": "replacement"}]
            db.sql("DROP TABLE docs")
            assert int(db.sql("SHOW STATUS LIKE 'docstore_cache_used_bytes'")["data"][0]["Value"]) == 0
        print(f"PASS {name}: adaptive={enabled}, stored-field replacement and cache cleanup")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("searchd", type=lambda value: str(Path(value).resolve()))
    args = parser.parse_args()
    cases = [
        ("default", "", True),
        ("enabled", "docstore_cache = 1\n", True),
        ("disabled", "docstore_cache = 0\n", False),
        ("disabled-large-budget", "docstore_cache = 0\ndocstore_cache_size = 512m\n", False),
        ("zero-budget", "docstore_cache_size = 0\n", False),
        ("enabled-zero-budget", "docstore_cache = 1\ndocstore_cache_size = 0\n", False),
    ]
    for case in cases:
        check(args.searchd, *case)
