# Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
# All rights reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License. You should have
# received a copy of the GPL license along with this program; if you
# did not, you can find it at http://www.gnu.org/

"""RT differential checks against a disposable daemon; leaves its fixtures intact."""

import argparse
import json
import re
import urllib.error
import urllib.parse
import urllib.request


def check_sort_bound_scan(sql, table, storage):
    sql(f"CREATE TABLE {table} (ts bigint, bucket int, shuffled int, rare int, peak int) engine='{storage}'")
    # The last columnar parent has just one partial leaf.
    docs = 64 * 1024 + 123 if storage == "columnar" else 32768
    for start in range(0, docs, 1024):
        peak = int(start == 4 * 1024)
        rows = [f"({i + 1},{i // 300 - 55},{i % 2},{i % 31},{int(i % 96 == 0)},{peak})" for i in range(start, min(start + 1024, docs))]
        sql(f"INSERT INTO {table} (id,ts,bucket,shuffled,rare,peak) VALUES " + ",".join(rows))

    def hinted(query, hint):
        rows, meta = sql(query + " /*+ " + hint + " */;SHOW META")
        info = {r["Variable_name"]: r["Value"] for r in meta["data"]}
        return rows["data"], info.get("index", ""), info.get("warning", "")

    query = f"SELECT id,ts FROM {table} WHERE rare=1 ORDER BY shuffled DESC,id ASC LIMIT 20,30 OPTION threads=1,max_matches=50"
    ram_rows, plan, warning = hinted(query, "SortBound(shuffled)")
    assert ram_rows == sql(query)[0]["data"] and ram_rows, ram_rows
    assert "SortBound" not in plan and not warning, (plan, warning)

    sql(f"FLUSH RAMCHUNK {table}")

    def compare(order, predicate="", expect_scan=True, hint=""):
        query = f"SELECT id,ts FROM {table} {predicate} ORDER BY {order} LIMIT 20,30 OPTION threads=1,max_matches=50"
        rows, meta = sql(query + (f" /*+ {hint} */" if hint else "") + ";SHOW META")
        expected, exact = sql(query + ",cutoff=0;SHOW META")
        assert rows["data"] == expected["data"], (storage, order, predicate)
        info = {r["Variable_name"]: r["Value"] for r in meta["data"]}
        total = int(next(r["Value"] for r in exact["data"] if r["Variable_name"] == "total_found"))
        if info["total_relation"] == "eq":
            assert int(info["total_found"]) == total, info
        else:
            assert info["total_relation"] == "gte" and 50 <= int(info["total_found"]) <= total, info
        if expect_scan is None:
            control = next((r["Value"] for r in exact["data"] if r["Variable_name"] == "index"), "")
            if "rare:SecondaryIndex" in control:
                assert "rare:SecondaryIndex" in info.get("index", ""), info
        elif storage == "rowwise" or not expect_scan:
            assert ("SortBound" in info.get("index", "")) == expect_scan, info
        return info

    plain = compare("ts DESC,id ASC")
    if "SortBound" in plain.get("index", ""):
        assert plain["total_relation"] == "gte" and int(plain["total_found"]) < docs, plain
    filtered = compare("ts DESC,id ASC", f"WHERE bucket=1 AND ts BETWEEN -55 AND {(docs - 1) // 300 - 55}")
    assert ("SortBound" in plain.get("index", "")) == ("SortBound" in filtered.get("index", "")), filtered
    compare("ts ASC,id DESC")
    compare("shuffled DESC,id ASC", expect_scan=False)
    if storage == "columnar":
        # Only leaf 4 contains ones. An evenly spaced sample misses it.
        info = compare("peak DESC,id ASC", "WHERE bucket=1")
        assert "peak:SortBoundColumnar" in info.get("index", "") and info["total_relation"] == "gte", info
    # Every block contains the maximum shuffled value, so a forced scan must visit them all.
    tied = compare("shuffled DESC,id ASC", hint="SortBound(shuffled)")
    assert tied["total_relation"] == "eq", tied
    if "SortBound" in plain.get("index", ""):
        assert "SortBound" in tied.get("index", ""), tied
    # More than 50 matches, but fetching the rare rows from an SI costs less than scanning by ts.
    compare("ts DESC,id ASC", "WHERE rare=1", expect_scan=None)

    disabled, plan, warning = hinted(query, "NO_SortBound(shuffled)")
    assert "SortBound" not in plan and not warning, (plan, warning)
    forced, plan, warning = hinted(query, "SortBound(shuffled)")
    assert forced == disabled
    assert "shuffled:SortBound" in plan and not warning, (plan, warning)

    combined, plan, warning = hinted(query, "SecondaryIndex(rare) SortBound(shuffled)")
    assert combined == disabled and "SortBound" not in plan, plan
    assert "explicit filter iterator hint" in warning and "filter not found" not in warning, warning
    repeated = hinted(query, "SecondaryIndex(rare) SortBound(shuffled) SecondaryIndex(rare)")
    assert repeated == (combined, plan, warning), repeated
    sql(query + " /*+ SortBound(ts) SortBound(shuffled) */", "conflicting sort-bound hints")
    sql(query + " /*+ SecondaryIndex(rare) SortBound(rare) NO_SecondaryIndex(rare) */", "conflicting hints")

    wrong, plan, warning = hinted(query, "SortBound(ts)")
    assert wrong == disabled and "SortBound" not in plan
    assert "primary ORDER BY attribute" in warning, warning
    cutoff, plan, warning = hinted(query + ",cutoff=0", "SortBound(shuffled)")
    assert cutoff == disabled and "automatic cutoff" in warning and "SortBound" not in plan, (plan, warning)

    common = f"SELECT id FROM {table} WHERE bucket=1 ORDER BY ts DESC,id ASC LIMIT 10 OPTION threads=1"
    disabled, plan, warning = hinted(common, "NO_SortBound(ts)")
    assert "SortBound" not in plan and not warning, (plan, warning)
    other, plan, warning = hinted(common, "NO_SortBound(shuffled)")
    assert other == disabled and not warning
    if "SortBound" in plain.get("index", ""):
        assert "ts:SortBound" in plan, plan

    sql(f"DELETE FROM {table} WHERE id IN (1,2,300,{docs})")
    info = compare("ts ASC,id DESC", "WHERE bucket=1")
    if "SortBound" in info.get("index", ""):
        assert info["total_relation"] == "gte", info

    sql(f"INSERT INTO {table} (id,ts,bucket,shuffled,rare) VALUES ({docs + 1},1000,1,31,1)")
    mixed, _, warning = hinted(common, "SortBound(ts)")
    assert mixed == sql(common)[0]["data"] and int(mixed[0]["id"]) == docs + 1, mixed
    assert "RT RAM segments" not in warning, warning


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", required=True, help="Private daemon HTTP endpoint")
    parser.add_argument("--prefix", default="postings_m2")
    parser.add_argument("--access", choices=["mmap", "file"], default="mmap")
    parser.add_argument("--field-lengths", choices=["0", "1"], default=None)
    parser.add_argument("--output", required=True, help="Result JSON outside the source tree")
    args = parser.parse_args()
    assert re.fullmatch(r"[a-z][a-z0-9_]*", args.prefix)
    endpoint = args.url.rstrip("/") + "/sql?mode=raw"
    checks = []

    def sql(query, expected_error=None):
        request = urllib.request.Request(endpoint, data=urllib.parse.urlencode({"query": query}).encode())
        try:
            result = json.loads(urllib.request.urlopen(request, timeout=60).read())
        except urllib.error.HTTPError as error:
            result = [json.loads(error.read())]
        errors = [item["error"] for item in result if item.get("error")]
        if expected_error:
            assert expected_error in str(errors), (query, result)
        else:
            assert not errors, (query, result)
        return result

    # CREATE fails if a name already exists. Never reuse/drop a pre-existing table.
    sql("CREATE TABLE " + args.prefix + "_invalid_crc (body text) postings_format='block' dict='crc'",
        "postings_format=block requires dict=keywords or dict=keywords_32k")
    checks.append({"case": "dictionary_policy", "stage": "reject_create_crc"})
    for case in ["wide", "columnar", "hitless", "keywords32k"]:
        fields = ["title", "body"] + (["f" + str(i) for i in range(34)] if case == "wide" else [])
        options = {"wide": "min_prefix_len='2'", "columnar": "engine='columnar'",
                   "hitless": "hitless_words='all'", "keywords32k": "dict='keywords_32k'"}[case]
        tables = [args.prefix + "_" + case + "_" + fmt for fmt in ["legacy", "block"]]
        for table, fmt in zip(tables, ["legacy", "block"]):
            sql("CREATE TABLE " + table + " (" + ",".join(f + " text" for f in fields) +
                ",grp int) postings_format='" + fmt + "' " + options +
                (" index_field_lengths='" + args.field_lengths + "'" if args.field_lengths else "") +
                " access_doclists='" + args.access + "' access_hitlists='" + args.access + "'")
            columns = sql("DESCRIBE " + table)[0]["data"]
            lengths = [column for column in columns if column["Type"] == "tokencount"]
            assert bool(lengths) == (args.field_lengths == "1"), (table, columns)
            checks.append({"case": case, "stage": "field_lengths", "enabled": bool(lengths)})

        sql("ALTER TABLE " + tables[1] + " postings_format='block' dict='crc'",
            "postings_format=block requires dict=keywords or dict=keywords_32k")
        checks.append({"case": case, "stage": "reject_alter_crc"})

        queries = [
            "SELECT count(*) FROM {} WHERE MATCH('common')",
            "SELECT count(*) FROM {} WHERE MATCH('common | rare')",
            "SELECT count(*) FROM {} WHERE MATCH('common alpha')",
            "SELECT count(*) FROM {} WHERE MATCH('@body alpha')",
            """SELECT count(*) FROM {} WHERE MATCH('"alpha beta"')""",
            "SELECT id,weight() FROM {} WHERE MATCH('common | rare') ORDER BY weight() DESC,id ASC LIMIT 50 OPTION ranker=bm25",
            "SELECT id,weight() FROM {} WHERE MATCH('alpha beta') ORDER BY weight() DESC,id ASC LIMIT 50 OPTION ranker=proximity_bm25",
            "SELECT id FROM {} WHERE MATCH('common | rare') AND grp=3 ORDER BY id ASC LIMIT 500 OPTION ranker=none",
            "SELECT grp,count(*) FROM {} WHERE MATCH('common') GROUP BY grp ORDER BY grp ASC",
            "SELECT count(*) FROM {} WHERE MATCH('@body (alpha | beta)')",
            "SELECT count(*) FROM {} WHERE MATCH('@body (common alpha)') AND grp=3",
            "SELECT id FROM {} WHERE MATCH('@body (alpha | beta)') ORDER BY id DESC LIMIT 50 OPTION ranker=none",
            "SELECT id FROM {} WHERE MATCH('rare common') ORDER BY id ASC LIMIT 50 OPTION ranker=none",
            "SELECT count(*) FROM {} WHERE MATCH('absent')",
        ]

        queries += ["SELECT id,weight() FROM {} WHERE MATCH('" + match +
                    "') ORDER BY id ASC OPTION ranker=proximity_bm25"
                    for match in ["singlerow", "@title singlerow", "sharedonce", '"singlerow sharedonce"', "repeatrow", '"repeatrow repeatrow"', "@title repeatrow", "multichunk"]]
        if case == "wide":
            queries += ["SELECT id FROM {} WHERE MATCH('singler*') ORDER BY id ASC",
                        "SELECT id FROM {} WHERE MATCH('@f33 upperfield') ORDER BY id ASC",
                        "SELECT id FROM {} WHERE MATCH('@f33 repeat*') ORDER BY id ASC",
                        "SELECT id,weight() FROM {} WHERE MATCH('mixedrare*') ORDER BY id ASC OPTION ranker=proximity_bm25"]

        # Flat scored AND exercises the block candidate/payload path. Compare
        # complete small result sets, scores and hit-based rankers across formats.
        for match in ["common alpha beta", "@title alpha @body beta", "rare common beta", "alpha alpha beta",
                      "common absent", "(alpha beta) | rare", "common alpha !rare"]:
            for ranker in ["bm25", "proximity_bm25", "wordcount", "expr('sum(lcs*user_weight)+bm25')"]:
                queries.append("SELECT id,weight() FROM {} WHERE MATCH('" + match +
                               "') ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,threads=1,ranker=" + ranker)

        queries += [
            "SELECT id,weight() FROM {} WHERE MATCH('common alpha beta') AND id BETWEEN 120 AND 900 ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,ranker=bm25",
            "SELECT id,weight() FROM {} WHERE MATCH('common alpha beta') AND grp=3 ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,ranker=proximity_bm25",
        ]
        if case == "wide":
            queries += ["SELECT count(*) FROM {} WHERE MATCH('@f33 common')",
                        "SELECT count(*) FROM {} WHERE MATCH('comm*')"]

        for count in [1, 2, 3, 4, 8, 16, 17]:
            term = "short" + str(count)
            for match in [term, "common " + term, "rare | " + term, "@body " + term]:
                queries.append("SELECT id,weight() FROM {} WHERE MATCH('" + match +
                               "') ORDER BY id ASC LIMIT 2000 OPTION ranker=bm25")

        def compare(stage, expected_docs):
            for query in queries:
                a, b = [sql(query.format(table))[0]["data"] for table in tables]
                assert a == b, (case, stage, query, a, b)
                checks.append({"case": case, "stage": stage, "query": query})
            for table in tables:
                result = sql("SELECT count(*) FROM " + table + " WHERE MATCH('common')")[0]["data"]
                assert result == [{"count(*)": expected_docs}], (table, stage, result)
            print(case, stage, "passed", flush=True)

        for start in range(1, 1026, 128):
            values = []
            for i in range(start, min(start + 128, 1026)):
                texts = ["common alpha " + ("rare" if i % 127 == 0 else "beta"),
                         "common " + "alpha beta " * (i % 4)]
                texts[1] += " " + " ".join("short" + str(count) for count in [1, 2, 3, 4, 8, 16, 17] if i <= count)
                texts += ["wide common" if f == i % 34 else "" for f in range(len(fields) - 2)]
                if i in (2, 800):
                    texts[0] += (" singlerow" if i == 2 else "") + " sharedonce"
                if i in (2, 800):
                    texts[0] += " multichunk multichunk"
                if i == 800:
                    texts[0] += " repeatrow repeatrow"
                    texts[-1] += " repeatrow"
                if case == "wide" and i in (2, 3, 800):
                    texts[0] += " mixedrarecommon"
                    if i == 800:
                        texts[-1] += " upperfield mixedrareone mixedraremulti mixedraremulti"
                values.append("(" + str(i) + "," + ",".join("'" + x + "'" for x in texts) + "," + str(i % 9) + ")")
            for table in tables:
                sql("INSERT INTO " + table + " (id," + ",".join(fields) + ",grp) VALUES " + ",".join(values))
            if start == 513:
                compare("ram", 640)
                for table in tables:
                    sql("FLUSH RAMCHUNK " + table)

        compare("ram_and_disk", 1025)
        for table in tables:
            sql("DELETE FROM " + table + " WHERE id IN (1,129,1025)")
            sql("FLUSH RAMCHUNK " + table)
        compare("disk", 1022)
        for table in tables:
            sql("ALTER TABLE " + table + " ADD COLUMN added int")
        compare("alter_attribute", 1022)
        for table, fmt in zip(tables, ["block", "legacy"]):
            sql("ALTER TABLE " + table + " postings_format='" + fmt + "'", "changing postings_format")
        for table in tables:
            sql("OPTIMIZE TABLE " + table + " OPTION sync=1,cutoff=1")
            status = sql("SHOW TABLE " + table + " STATUS")[0]["data"]
            assert [x["Value"] for x in status if x["Variable_name"] == "disk_chunks"] == ["1"]
        compare("merged", 1022)

    for storage in ["rowwise", "columnar"]:
        check_sort_bound_scan(sql, args.prefix + "_sort_" + storage, storage)
        checks.append({"case": "sort_bound_scan", "storage": storage})

    with open(args.output, "w", encoding="utf-8", newline="\n") as output:
        json.dump(checks, output, indent=2)
        output.write("\n")
    print(len(checks), "integration checks passed")


if __name__ == "__main__":
    main()
