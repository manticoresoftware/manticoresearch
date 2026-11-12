# Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
# All rights reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License. You should have
# received a copy of the GPL license along with this program; if you
# did not, you can find it at http://www.gnu.org/

"""Check competitive sort pruning against exhaustive execution on a disposable RT daemon.

Requires query caching disabled. Creates a new table under --prefix, retains it
for inspection, and writes rows and metadata to --output. Use --expect-si with
an SI-enabled runtime to verify that both alternative range sources were used.
"""

import argparse
import json
import re
import urllib.parse
import urllib.request
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", required=True)
    parser.add_argument("--prefix", default="sortbound")
    parser.add_argument("--output", required=True)
    parser.add_argument("--expect-si", action="store_true")
    args = parser.parse_args()
    assert re.fullmatch(r"[a-z][a-z0-9_]*", args.prefix)
    endpoint = args.url.rstrip("/") + "/sql?mode=raw"
    table = args.prefix + "_block"
    checks = []

    def sql(query):
        Path(args.output + ".last-sql").write_text(query, encoding="utf-8")
        request = urllib.request.Request(endpoint, data=urllib.parse.urlencode({"query": query}).encode())
        result = json.loads(urllib.request.urlopen(request, timeout=120).read())
        assert all(not item.get("error") and not item.get("warning") for item in result), (query, result)
        return result

    def meta(result):
        return {item["Variable_name"]: item["Value"] for item in result[-1]["data"]}

    def compare(query):
        exact = sql(query.replace(" OPTION ", " OPTION cutoff=0,") + ";SHOW META")
        actual = sql(query + ";SHOW META")
        assert actual[0]["data"] == exact[0]["data"], (query, actual, exact)
        actual_meta, exact_meta = meta(actual), meta(exact)
        assert exact_meta["total_relation"] == "eq", (query, exact_meta)
        assert int(actual_meta["total_found"]) <= int(exact_meta["total_found"]), (query, actual_meta, exact_meta)
        checks.append(dict(query=query, rows=actual[0]["data"], meta=actual_meta, exact=exact_meta))
        Path(args.output).write_text(json.dumps(checks, indent=2) + "\n", encoding="utf-8")

    status = {item["Counter"]: item["Value"] for item in sql("SHOW STATUS")[0]["data"]}
    assert status["qcache_max_bytes"] == "0", "Disable query caching before running this test"
    sql(f"CREATE TABLE {table}(body text, ts bigint) postings_format='block'")
    for chunk in range(2):
        for start in range(0, 8192, 512):
            rows = []
            for i in range(start, start + 512):
                stamp = 10000 + i % 11 if i < 128 else i % 100 - 50
                body = "failed to place order" if i % 4 else "failed to cancel the order"
                rows.append(f"({chunk * 8192 + i + 1},'{body}',{stamp})")
            sql(f"INSERT INTO {table}(id,body,ts) VALUES " + ",".join(rows))
        sql("FLUSH RAMCHUNK " + table)
    sql(f"DELETE FROM {table} WHERE id IN (2,3,4,6,7,8,10,11,12)")

    phrase = "MATCH('\"failed to place order\"')"
    for threads in [1, 4]:
        for order in ["asc", "desc"]:
            for method in ["pq", "kbuffer"]:
                for hint in ["", " /*+ NO_SECONDARYINDEX(ts) */"]:
                    for ranker in ["none", "proximity_bm25"]:
                        compare(f"SELECT id,ts,weight() FROM {table} WHERE {phrase} AND ts>=-1000000 "
                                f"ORDER BY ts {order},id asc LIMIT 7,20 "
                                f"OPTION threads={threads},ranker={ranker},sort_method={method}" + hint)

    # Single-term scored and membership roots must both prune, including field masks.
    for threads in [1, 4]:
        for order in ["asc", "desc"]:
            for method in ["pq", "kbuffer"]:
                for hint in ["", " /*+ NO_SECONDARYINDEX(ts) */"]:
                    for ranker in ["none", "proximity_bm25", "bm25", "expr('sum(lcs)*1000+bm25')"]:
                        compare(f"SELECT id,ts,weight() FROM {table} WHERE MATCH('@body failed') AND ts>=-1000000 "
                                f"ORDER BY ts {order},id asc LIMIT 7,20 "
                                f"OPTION threads={threads},ranker={ranker},sort_method={method}" + hint)
                        assert "SortBound" in checks[-1]["meta"].get("index", ""), checks[-1]

    # Branches must independently retain the same eligible rows, including duplicates,
    # phrase hits and asymmetric posting density. Verify both OR execution paths.
    for expr in ["failed | place | missing", "failed | failed", '\"failed to place order\" | cancel',
                 "(failed place) | cancel", "missing | order", "failed | ^order"]:
        for threads in [1, 4]:
            for order in ["asc", "desc"]:
                for ranker in ["none", "bm25", "proximity_bm25", "expr('sum(lcs)*1000+bm25')"]:
                    for hint in ["", " /*+ NO_SECONDARYINDEX(ts) */"]:
                        compare(f"SELECT id,ts,weight() FROM {table} WHERE MATCH('{expr}') "
                                f"ORDER BY ts {order},id asc LIMIT 7,20 "
                                f"OPTION threads={threads},ranker={ranker}" + hint)
    for method in ["pq", "kbuffer"]:
        for ranker in ["none", "bm25"]:
            compare(f"SELECT id,ts,weight() FROM {table} WHERE MATCH('failed | cancel') "
                    f"ORDER BY ts desc,id asc LIMIT 7,20 OPTION threads=4,ranker={ranker},sort_method={method}")

    # Better keys in a live RAM segment must survive disk-chunk pruning and merging.
    rows = [f"({20000 + i},'failed to place order',{20000 + i})" for i in range(1, 65)]
    sql(f"INSERT INTO {table}(id,body,ts) VALUES " + ",".join(rows))
    for threads in [1, 4]:
        for order in ["asc", "desc"]:
            compare(f"SELECT id,ts AS stamp,weight() FROM {table} WHERE {phrase} "
                    f"ORDER BY stamp {order},id asc LIMIT 7,20 OPTION threads={threads}")

    for threads in [1, 4]:
        for ranker in ["none", "bm25"]:
            compare(f"SELECT id,ts AS stamp,weight() FROM {table} WHERE MATCH('failed') "
                    f"ORDER BY stamp desc,id asc LIMIT 7,20 OPTION threads={threads},ranker={ranker}")

    # Separate fields force low-field-mask rejection; rare terms exercise short lists.
    fields = args.prefix + "_fields"
    sql(f"CREATE TABLE {fields}(body text, other text, ts bigint) postings_format='block'")
    for start in range(0, 5000, 500):
        rows = [f"({i+1},'{('error rare' if i%977==0 else 'error') if i%3 else 'other'}',"
                f"'error',{i%137})" for i in range(start, start+500)]
        sql(f"INSERT INTO {fields}(id,body,other,ts) VALUES " + ",".join(rows))
    sql("FLUSH RAMCHUNK " + fields)
    for term in ["@body error", "@other error", "error", "@body rare", "@body missing", "(@body error) | (@other error)", "rare | missing | other"]:
        for ranker in ["none", "proximity_bm25"]:
            compare(f"SELECT id,ts,weight() FROM {fields} WHERE MATCH('{term}') "
                    f"ORDER BY ts desc,id asc LIMIT 7 OPTION threads=1,ranker={ranker}")

    for expr in ["failed | place", '\"failed to place order\" | cancel']:
        compare(f"SELECT id,ts,weight() FROM {table} WHERE MATCH('{expr}') "
                f"ORDER BY ts desc,id asc LIMIT 7,20 OPTION threads=4")

    # Legacy membership OR uses the same union filter; ranked legacy terms fall back.
    legacy = args.prefix + "_legacy"
    sql(f"CREATE TABLE {legacy}(body text, ts bigint)")
    for start in range(0, 4096, 512):
        rows = [f"({i+1},'{('error failed' if i%3 else 'failed')}',{10000+i%11 if i<128 else i%100})"
                for i in range(start, start+512)]
        sql(f"INSERT INTO {legacy}(id,body,ts) VALUES " + ",".join(rows))
    sql("FLUSH RAMCHUNK " + legacy)
    for threads in [1, 4]:
        for ranker in ["none", "bm25"]:
            for hint in ["", " /*+ NO_SECONDARYINDEX(ts) */"]:
                compare(f"SELECT id,ts,weight() FROM {legacy} WHERE MATCH('error | failed') "
                        f"ORDER BY ts desc,id asc LIMIT 7,20 OPTION threads={threads},ranker={ranker}" + hint)
                if ranker == "none":
                    assert "SortBound" in checks[-1]["meta"].get("index", ""), checks[-1]
                else:
                    assert checks[-1]["meta"]["total_relation"] == "eq", checks[-1]

    fallbacks = [
        f"SELECT id,ts FROM {table} WHERE {phrase} AND ts>10000 ORDER BY ts desc,id asc LIMIT 20 OPTION threads=1 /*+ SECONDARYINDEX(ts) */",
        f"SELECT COUNT(*) FROM {table} WHERE {phrase} OPTION threads=4",
        f"SELECT ts,COUNT(*) FROM {table} WHERE {phrase} GROUP BY ts ORDER BY ts DESC LIMIT 20 OPTION threads=4",
    ]
    for query in fallbacks:
        compare(query)
        assert checks[-1]["meta"]["total_relation"] == "eq", checks[-1]

    for ranker in ["none", "bm25", "proximity_bm25"]:
        admitted = [item for item in checks if "MATCH('failed | place | missing')" in item["query"]
                    and f"ranker={ranker}" in item["query"]]
        assert any("SortBoundMinMax" in item["meta"].get("index", "") for item in admitted), admitted
        if args.expect_si:
            assert any("SortBoundSI" in item["meta"].get("index", "") for item in admitted), admitted

    assert any("SortBoundMinMax" in item["meta"].get("index", "") for item in checks)
    if args.expect_si:
        assert any("SortBoundSI" in item["meta"].get("index", "") for item in checks)
    assert any(item["meta"]["total_relation"] == "gte" for item in checks)
    print(len(checks), "sort-bound comparisons passed")


if __name__ == "__main__":
    main()
