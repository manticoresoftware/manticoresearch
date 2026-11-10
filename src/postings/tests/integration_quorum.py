# Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
# All rights reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License. You should have
# received a copy of the GPL license along with this program; if you
# did not, you can find it at http://www.gnu.org/

"""Compare quorum rows/scores across formats and optionally against an old binary.

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
    parser.add_argument("--prefix", default="quorum")
    parser.add_argument("--access", choices=["mmap", "file"], default="mmap")
    parser.add_argument("--stopwords", required=True, help="Daemon-side file containing 'the'")
    parser.add_argument("--output", required=True)
    parser.add_argument("--oracle", help="Result JSON captured with the baseline binary")
    parser.add_argument("--expect-windows", action="store_true", help="Assert native quorum selection on disk tables")
    args = parser.parse_args()
    assert re.fullmatch(r"[a-z][a-z0-9_]*", args.prefix)
    endpoint = args.url.rstrip("/") + "/sql?mode=raw"
    expected = json.loads(Path(args.oracle).read_text()) if args.oracle else None
    checks = []

    def sql(query, result_index=0):
        Path(args.output + ".last-sql").write_text(query)
        request = urllib.request.Request(endpoint, data=urllib.parse.urlencode({"query": query}).encode())
        result = json.loads(urllib.request.urlopen(request, timeout=60).read())
        assert all(not item.get("error") for item in result), (query, result)
        return result[result_index]

    matches = ['"alpha beta gamma common"/2', '"alpha beta gamma common"/3',
               '"alpha beta gamma common"/0.5', '"alpha beta gamma common"/0.75',
               '"alpha beta gamma common"/1', '"alpha beta gamma common"/4',
               '"alpha beta gamma common"/5', '"alpha beta absent missing"/2',
               '"rare absent missing common"/2', '"absent missing common"/2',
               '"alpha alpha beta gamma"/2', '"alpha alpha alpha beta"/3',
               '@title "alpha beta gamma common"/2', '@body "alpha beta gamma common"/3',
               '"^alpha beta gamma"/2', '"alpha beta$ gamma"/2',
               '@body[3] "alpha beta gamma"/2', '"alp* beta gamma"/2',
               '("alpha beta gamma"/2) | common', 'common ("alpha beta gamma"/2)',
               '"alpha the beta gamma"/2']
    for count in [8, 16, 32, 33]:
        words = ["alpha", "beta", "gamma", "common"] + ["missing" + str(i) for i in range(count-4)]
        for threshold in [2, 3, count-1]:
            matches.append('"' + ' '.join(words) + '"/' + str(threshold))
    rankers = ["none", "bm25", "proximity_bm25", "expr('sum(lcs*user_weight)+bm25')"]
    queries = ["SELECT id,weight() FROM {} WHERE MATCH('" + match +
               "') ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,threads=1,ranker=" + ranker
               for match in matches for ranker in rankers]
    queries += ["SELECT count(*) FROM {} WHERE MATCH('" + match + "') OPTION threads=1" for match in matches]
    queries += [
        "SELECT count(*) FROM {} WHERE MATCH('\"alpha beta gamma common\"/2') AND id BETWEEN 120 AND 390 OPTION threads=1",
        "SELECT id FROM {} WHERE MATCH('\"alpha beta gamma common\"/2') AND grp=3 ORDER BY id DESC LIMIT 25 OPTION ranker=none,threads=1",
    ]
    patterns = ["alpha beta gamma", "alpha alpha beta", "common alpha beta gamma",
                "beta alpha gamma", "alpha the beta gamma", "alpha beta alpha beta",
                "alpha gamma beta", "beta gamma", "common gamma alpha beta",
                "alpha beta " * 80, "common alpha", "gamma beta alpha"]

    for case in ["plain", "infix", "wide_stop", "hitless"]:
        fields = ["title", "body"] + (["f" + str(i) for i in range(34)] if case == "wide_stop" else [])
        options = ("min_infix_len='2'" if case == "infix" else "min_prefix_len='2'") + (" stopwords='" + args.stopwords + "'" if case == "wide_stop" else "")
        if case == "hitless":
            options += " hitless_words='all'"
        tables = [args.prefix + "_" + case + "_" + fmt for fmt in ["legacy", "block"]]
        for table, fmt in zip(tables, ["legacy", "block"]):
            sql("CREATE TABLE " + table + " (" + ",".join(f + " text" for f in fields) +
                ",grp int) postings_format='" + fmt + "' index_field_lengths='1' " + options +
                " access_doclists='" + args.access + "' access_hitlists='" + args.access + "'")

        case_queries = queries + ([
            "SELECT id,weight() FROM {} WHERE MATCH('@f33 common !@f33 alpha') ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,threads=1",
            "SELECT id,weight() FROM {} WHERE MATCH('@f33 alpha !@body gamma') ORDER BY id ASC LIMIT 2000 OPTION max_matches=2000,threads=1",
        ] if case == "wide_stop" else [])

        # The baseline expression ranker crashes on compound hitless queries.
        # Keep expression coverage on positional tables; hitless uses the old path.
        if case == "hitless":
            case_queries = [q for q in case_queries if 'ranker=expr' not in q]

        def compare(stage):
            if args.expect_windows and case in ["plain", "infix"] and stage in ["mixed", "disk", "merged"]:
                for table in tables:
                    result = sql("SELECT count(*) FROM " + table + " WHERE MATCH('\"alpha beta gamma common\"/2') OPTION threads=1; SHOW META", 1)
                    meta = {x["Variable_name"]: x["Value"] for x in result["data"]}
                    assert "WindowQuorumCount" in meta["index"], meta
            for query in case_queries:
                results = [sql(query.format(table)) for table in tables]
                rows = [r["data"] for r in results]
                assert rows[0] == rows[1], (case, stage, query, rows)
                item = dict(case=case, stage=stage, query=query, rows=rows[0],
                            warnings=[r.get("warning", "") for r in results])
                if expected is not None:
                    assert item == expected[len(checks)], (len(checks), case, stage, query, rows[0][:5], expected[len(checks)]["rows"][:5])
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
    print(len(checks), "quorum checks passed")


if __name__ == "__main__":
    main()
