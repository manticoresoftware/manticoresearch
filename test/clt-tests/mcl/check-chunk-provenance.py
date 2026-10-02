#!/usr/bin/env python3

import json
import sys


def parse_tuple(value):
    result = tuple(map(int, value.split()))
    assert len(result) == 3, result
    return result


def validate_range(text, values, require_phrase):
    slot, start, end = values
    raw = text.encode("utf-8")
    assert slot >= 0 and 0 <= start < end <= len(raw), (values, len(raw))
    chunk = raw[start:end].decode("utf-8")
    if require_phrase:
        assert "target phrase" in chunk, (values, chunk)


def expected_chunk(text, values):
    _, start, end = values
    return text.encode("utf-8")[start:end].decode("utf-8")


def validate_highlight(highlighted, chunk, tagged):
    if tagged:
        assert highlighted.replace("<b>", "").replace("</b>", "") == chunk, (highlighted, chunk)
        tags = ("<b>target phrase</b>", "<b>target</b> <b>phrase</b>")
        assert any(tag in highlighted for tag in tags), highlighted
    else:
        assert highlighted == chunk and "<b>" not in highlighted, (highlighted, chunk)


def read_tsv_rows():
    return [line.split("\t") for line in sys.stdin.read().splitlines() if line]


mode = sys.argv[1]
if mode == "range":
    values = parse_tuple(sys.argv[3])
    validate_range(sys.argv[2], values, True)
    print("CHUNK_RANGE_OK")
elif mode in ("http", "hybrid"):
    hit = json.load(sys.stdin)["hits"]["hits"][0]
    values = tuple(hit["_knn_chunk"][key] for key in ("index", "start", "end"))
    if mode == "http":
        expected = parse_tuple(sys.argv[2])
        assert values == expected, (values, expected)
        print("CHUNK_HTTP_OK")
    else:
        validate_range(sys.argv[2], values, False)
        print("CHUNK_HYBRID_OK")
elif mode == "distributed":
    sources = {
        int(sys.argv[i]): sys.argv[i + 1]
        for i in range(2, len(sys.argv), 2)
    }
    rows = [line for line in sys.stdin.read().splitlines() if line.strip()]
    assert len(rows) == len(sources), (rows, sources)
    seen = set()
    for row in rows:
        docid, slot, start, end = map(int, row.split())
        assert docid in sources and docid not in seen, row
        validate_range(sources[docid], (slot, start, end), True)
        seen.add(docid)
    assert seen == set(sources), (seen, sources)
    print("DISTRIBUTED_CHUNK_PROVENANCE_OK")
elif mode == "highlight":
    text, values, highlighted, tagged = sys.argv[2:]
    chunk = expected_chunk(text, parse_tuple(values))
    validate_highlight(highlighted, chunk, tagged == "tagged")
    print("CHUNK_HIGHLIGHT_OK")
elif mode == "json-highlight":
    data = json.load(sys.stdin)
    if "error" in data:
        raise AssertionError(f"unexpected JSON error: {data['error']}")
    hits = data.get("hits", {}).get("hits", [])
    if len(hits) != 1:
        raise AssertionError(f"expected exactly one hit, got {len(hits)}")
    hit = hits[0]
    values = tuple(hit["_knn_chunk"][key] for key in ("index", "start", "end"))
    chunk = expected_chunk(sys.argv[2], values)
    highlighted = hit["highlight"]["body"][0]
    validate_highlight(highlighted, chunk, len(sys.argv) > 3 and sys.argv[3] == "tagged")
    print("CHUNK_JSON_HIGHLIGHT_OK")
elif mode == "json-error":
    response = json.load(sys.stdin)
    error = response.get("error")
    assert isinstance(error, str), response
    assert "hits" not in response, response
    assert response.get("success") is not True, response
    assert sys.argv[2] in error, (error, sys.argv[2])
elif mode == "json-success":
    response = json.load(sys.stdin)
    assert "error" not in response, response
    hits = response["hits"]["hits"]
    expected_ids = [int(value) for value in sys.argv[2:]]
    actual_ids = [int(hit["_id"]) for hit in hits]
    assert len(hits) == len(expected_ids), (actual_ids, expected_ids)
    assert actual_ids == expected_ids, (actual_ids, expected_ids)
    print("CHUNK_JSON_SUCCESS_OK")
elif mode == "distributed-highlight":
    tagged = sys.argv[2] == "tagged"
    sources = {
        int(sys.argv[i]): sys.argv[i + 1]
        for i in range(3, len(sys.argv), 2)
    }
    rows = [line.split("\t", 4) for line in sys.stdin.read().splitlines() if line]
    assert len(rows) == len(sources), (rows, sources)
    seen = set()
    for docid, slot, start, end, highlighted in rows:
        docid = int(docid)
        assert docid in sources and docid not in seen, (docid, sources, seen)
        values = (int(slot), int(start), int(end))
        chunk = expected_chunk(sources[docid], values)
        validate_highlight(highlighted, chunk, tagged)
        seen.add(docid)
    assert seen == set(sources), (seen, sources)
    print("DISTRIBUTED_HYBRID_CHUNK_HIGHLIGHT_OK" if tagged else "DISTRIBUTED_CHUNK_HIGHLIGHT_OK")
elif mode == "grouped-highlight":
    expected_id = int(sys.argv[2])
    source = sys.argv[3]
    rows = read_tsv_rows()
    assert len(rows) == 1 and len(rows[0]) == 6, rows
    docid, group_count, slot, start, end, highlighted = rows[0]
    assert int(docid) == expected_id, (docid, expected_id)
    assert int(group_count) == 2, group_count
    values = (int(slot), int(start), int(end))
    chunk = expected_chunk(source, values)
    validate_range(source, values, True)
    validate_highlight(highlighted, chunk, False)
    print("GROUPED_CHUNK_HIGHLIGHT_OK")
elif mode == "format-tuples":
    column_count = int(sys.argv[2])
    rows = read_tsv_rows()
    assert rows and all(len(row) == column_count for row in rows), rows
    for row in rows:
        print(" ".join(row))
elif mode == "valid-tuple":
    rows = read_tsv_rows()
    assert len(rows) == 1 and len(rows[0]) == 3, rows
    slot, start, end = map(int, rows[0])
    assert slot >= 0 and 0 <= start < end, rows[0]
    print(sys.argv[2])
elif mode == "json-no-provenance":
    response = json.load(sys.stdin)
    assert "error" not in response, response
    hits = response["hits"]["hits"]
    expected_ids = [int(value) for value in sys.argv[2:]]
    actual_ids = [int(hit["_id"]) for hit in hits]
    assert len(actual_ids) == len(expected_ids), (actual_ids, expected_ids)
    assert set(actual_ids) == set(expected_ids), (actual_ids, expected_ids)
    assert all("_knn_chunk" not in hit for hit in hits), hits
    print("LEGACY_HTTP_OK")
else:
    raise SystemExit(f"unknown mode: {mode}")
