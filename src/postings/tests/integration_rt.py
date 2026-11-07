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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", required=True, help="Private daemon HTTP endpoint")
    parser.add_argument("--prefix", default="postings_m2")
    parser.add_argument("--access", choices=["mmap", "file"], default="mmap")
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
                ",grp int) postings_format='" + fmt + "' index_field_lengths='1' " + options +
                " access_doclists='" + args.access + "' access_hitlists='" + args.access + "'")

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
            "SELECT count(*) FROM {} WHERE MATCH('absent')",
        ]
        if case == "wide":
            queries += ["SELECT count(*) FROM {} WHERE MATCH('@f33 common')",
                        "SELECT count(*) FROM {} WHERE MATCH('comm*')"]

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
                texts += ["wide common" if f == i % 34 else "" for f in range(len(fields) - 2)]
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

    with open(args.output, "w", encoding="utf-8", newline="\n") as output:
        json.dump(checks, output, indent=2)
        output.write("\n")
    print(len(checks), "integration checks passed")


if __name__ == "__main__":
    main()
