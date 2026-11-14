# Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
# All rights reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License. You should have
# received a copy of the GPL license along with this program; if you
# did not, you can find it at http://www.gnu.org/

"""Compare positional rows/scores across formats and optionally against an old binary.

Run against disposable RT daemons. Save --output with the old binary, then pass
that file as --oracle when running the new binary against fresh tables.
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
    parser.add_argument("--prefix", default="positional")
    parser.add_argument("--access", choices=["mmap", "file"], default="mmap")
    parser.add_argument("--stopwords", required=True, help="Daemon-side file containing 'the'")
    parser.add_argument("--output", required=True)
    parser.add_argument("--oracle", help="Result JSON captured with the baseline binary")
    args = parser.parse_args()
    assert re.fullmatch(r"[a-z][a-z0-9_]*", args.prefix)
    endpoint = args.url.rstrip("/") + "/sql?mode=raw"
    expected = json.loads(Path(args.oracle).read_text()) if args.oracle else None
    checks = []

    def sql(query):
        request = urllib.request.Request(endpoint, data=urllib.parse.urlencode({"query": query}).encode())
        result = json.loads(urllib.request.urlopen(request, timeout=60).read())
        assert all(not item.get("error") for item in result), (query, result)
        return result[0]

    matches = [
        '"alpha beta"', '"common alpha beta"', '"common alpha beta gamma"',
        '"alpha alpha"', '"alpha beta alpha"', '"alpha absent"', '"rare alpha"',
        '"alpha beta"~3', '"alpha beta gamma"~5', '"alpha alpha"~3',
        'alpha NEAR/3 beta', 'alpha NEAR/3 beta NEAR/3 gamma',
        'common NEAR/5 alpha NEAR/5 beta NEAR/5 gamma', 'alpha NEAR/3 alpha',
        '@title "alpha beta"', '@body "common alpha beta"',
        '@title alpha NEAR/3 @body beta NEAR/3 @body gamma',
        '@body alpha NEAR/3 @title beta NEAR/3 @body gamma',
        '"alpha the beta"', '"alpha the beta"~4',
        # Existing tree fallback: nesting, expansion, anchors and field limits.
        '(alpha | gamma) NEAR/3 beta', '"alpha beta" NEAR/3 gamma',
        '"alp* beta"', '"^alpha beta"', '"alpha beta$"', '@body[3] "alpha beta"',
        '("alpha beta" | "common gamma") !rare',
    ]
    rankers = ["none", "bm25", "proximity_bm25", "expr('sum(lcs*user_weight)+bm25')"]
    queries = ["SELECT id,weight() FROM {} WHERE MATCH('" + match +
               "') ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,threads=1,ranker=" + ranker
               for match in matches for ranker in rankers]
    queries += [
        "SELECT count(*) FROM {} WHERE MATCH('\"alpha beta\"') OPTION threads=1",
        "SELECT id,weight() FROM {} WHERE MATCH('\"alpha beta\"') AND id BETWEEN 120 AND 390 ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,threads=1",
        "SELECT id,weight() FROM {} WHERE MATCH('alpha NEAR/3 beta') AND grp=3 ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000",
        "SELECT id,weight() FROM {} WHERE MATCH('\"alpha beta\" | \"alpha beta\"') ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,threads=1",
    ]
    patterns = ["alpha beta gamma", "alpha alpha beta", "common alpha beta gamma",
                "beta alpha gamma", "alpha the beta gamma", "alpha beta alpha beta",
                "alpha gamma beta", "beta gamma", "common gamma alpha beta",
                "alpha beta " * 80, "common alpha", "gamma beta alpha"]

    for case in ["plain", "wide_stop", "hitless"]:
        fields = ["title", "body"] + (["f" + str(i) for i in range(34)] if case == "wide_stop" else [])
        options = "min_prefix_len='2'" + (" stopwords='" + args.stopwords + "'" if case == "wide_stop" else "")
        if case == "hitless":
            options += " hitless_words='all'"
        tables = [args.prefix + "_" + case + "_" + fmt for fmt in ["legacy", "block"]]
        for table, fmt in zip(tables, ["legacy", "block"]):
            sql("CREATE TABLE " + table + " (" + ",".join(f + " text" for f in fields) +
                ",grp int) postings_format='" + fmt + "' index_field_lengths='1' " + options +
                " access_doclists='" + args.access + "' access_hitlists='" + args.access + "'")

        case_queries = queries + ([
            "SELECT id,weight() FROM {} WHERE MATCH('@f33 \"alpha beta\"') ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,threads=1",
            "SELECT id,weight() FROM {} WHERE MATCH('@f33 alpha NEAR/3 @f33 beta') ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,threads=1",
        ] if case == "wide_stop" else [])

        # The baseline expression ranker crashes on this hitless compound query.
        # Keep the case for positional tables; hitless terms use the old fallback.
        if case == "hitless":
            case_queries = [q for q in case_queries if not ('!rare' in q and 'ranker=expr' in q)]

        def compare(stage):
            for query in case_queries:
                results = [sql(query.format(table)) for table in tables]
                rows = [r["data"] for r in results]
                assert rows[0] == rows[1], (case, stage, query, rows)
                item = dict(case=case, stage=stage, query=query, rows=rows[0],
                            warnings=[r.get("warning", "") for r in results])
                if expected is not None:
                    assert item == expected[len(checks)], (len(checks), item, expected[len(checks)])
                checks.append(item)
            Path(args.output).write_text(json.dumps(checks, indent=2) + "\n")
            print(case, stage, len(checks), "passed", flush=True)

        for start in range(1, 514, 64):
            values = []
            for i in range(start, min(start + 64, 514)):
                texts = [patterns[i % len(patterns)], patterns[(i * 7 + 3) % len(patterns)]]
                if i % 31 == 0:
                    texts[0] = "rare " + texts[0]
                texts += [patterns[i % len(patterns)] if f == 33 else "" for f in range(len(fields) - 2)]
                values.append("(" + str(i) + "," + ",".join("'" + x + "'" for x in texts) + "," + str(i % 9) + ")")
            for table in tables:
                sql("INSERT INTO " + table + " (id," + ",".join(fields) + ",grp) VALUES " + ",".join(values))
            if start == 193:
                compare("ram")
                for table in tables:
                    sql("FLUSH RAMCHUNK " + table)
        compare("mixed")
        for table in tables:
            sql("DELETE FROM " + table + " WHERE id IN (1,129,257,513)")
            sql("FLUSH RAMCHUNK " + table)
        compare("disk")
        for table in tables:
            sql("OPTIMIZE TABLE " + table + " OPTION sync=1,cutoff=1")
        compare("merged")

    if expected is not None:
        assert len(expected) == len(checks)
    print(len(checks), "positional checks passed")


if __name__ == "__main__":
    main()
