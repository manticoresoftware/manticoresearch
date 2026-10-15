#!/usr/bin/env python3
#
# Helpers for the KNN chunk highlighting CLT tests. HIGHLIGHT({knn_chunk=1},'field') must return the chunk of the
# source text whose vector matched the KNN query: a proper substring of the source that contains the query phrase.

import json
import sys

PHRASE = "target phrase"


def strip_tags(text):
    return text.replace("<b>", "").replace("</b>", "")


def check_chunk(source, highlighted, tagged, strict=True):
    chunk = strip_tags(highlighted) if tagged else highlighted
    assert chunk, "empty highlight"
    assert chunk in source, (chunk, source)
    # a short document is a single chunk, so only the long single-document checks insist on a proper substring
    assert not strict or len(chunk) < len(source), "highlight is the whole field, not a chunk: %r" % chunk
    assert PHRASE in chunk, (chunk, PHRASE)
    if tagged:
        tags = ("<b>target phrase</b>", "<b>target</b> <b>phrase</b>")
        assert any(tag in highlighted for tag in tags), highlighted
    else:
        assert "<b>" not in highlighted, highlighted
    return chunk


def read_lines():
    return [line for line in sys.stdin.read().splitlines() if line.strip()]


mode = sys.argv[1]
if mode == "chunk":
    # chunk SOURCE HIGHLIGHTED [tagged]: the highlight is a chunk of SOURCE; prints the plain chunk
    chunk = check_chunk(sys.argv[2], sys.argv[3], len(sys.argv) > 4 and sys.argv[4] == "tagged")
    print("CHUNK_OK")
elif mode == "same":
    # same EXPECTED ACTUAL: the chunk did not change
    assert sys.argv[2] == sys.argv[3], (sys.argv[2], sys.argv[3])
    print("CHUNK_SAME_OK")
elif mode == "whole":
    # whole SOURCE HIGHLIGHTED: no chunk restriction applied, the whole field came back
    assert strip_tags(sys.argv[3]) == sys.argv[2], (sys.argv[3], sys.argv[2])
    print("WHOLE_FIELD_OK")
elif mode == "json-highlight":
    # json-highlight SOURCE [tagged] < response.json
    data = json.load(sys.stdin)
    assert "error" not in data, data
    hits = data.get("hits", {}).get("hits", [])
    assert len(hits) == 1, hits
    highlighted = hits[0]["highlight"]["body"][0]
    check_chunk(sys.argv[2], highlighted, len(sys.argv) > 3 and sys.argv[3] == "tagged")
    print("CHUNK_JSON_HIGHLIGHT_OK")
elif mode == "json-error":
    # json-error MESSAGE < response.json
    response = json.load(sys.stdin)
    error = response.get("error")
    assert isinstance(error, str), response
    assert sys.argv[2] in error, (error, sys.argv[2])
    print("JSON_ERROR_OK")
elif mode == "json-success":
    # json-success ID... < response.json
    response = json.load(sys.stdin)
    assert "error" not in response, response
    actual_ids = [int(hit["_id"]) for hit in response["hits"]["hits"]]
    expected_ids = [int(value) for value in sys.argv[2:]]
    assert actual_ids == expected_ids, (actual_ids, expected_ids)
    print("JSON_SUCCESS_OK")
elif mode == "distributed-highlight":
    # distributed-highlight plain|tagged ID SOURCE [ID SOURCE ...] < "id\thighlight" rows
    tagged = sys.argv[2] == "tagged"
    sources = {int(sys.argv[i]): sys.argv[i + 1] for i in range(3, len(sys.argv), 2)}
    rows = [line.split("\t", 1) for line in read_lines()]
    assert len(rows) == len(sources), (rows, sources)
    seen = set()
    for docid, highlighted in rows:
        docid = int(docid)
        assert docid in sources and docid not in seen, (docid, sources, seen)
        check_chunk(sources[docid], highlighted, tagged, strict=False)
        seen.add(docid)
    assert seen == set(sources), (seen, sources)
    print("DISTRIBUTED_HYBRID_CHUNK_HIGHLIGHT_OK" if tagged else "DISTRIBUTED_CHUNK_HIGHLIGHT_OK")
elif mode == "grouped-highlight":
    # grouped-highlight ID SOURCE < "id\tcount\thighlight"
    rows = [line.split("\t", 2) for line in read_lines()]
    assert len(rows) == 1 and len(rows[0]) == 3, rows
    docid, group_count, highlighted = rows[0]
    assert int(docid) == int(sys.argv[2]), (docid, sys.argv[2])
    assert int(group_count) == 2, group_count
    check_chunk(sys.argv[3], highlighted, False, strict=False)
    print("GROUPED_CHUNK_HIGHLIGHT_OK")
else:
    raise SystemExit("unknown mode: %s" % mode)
