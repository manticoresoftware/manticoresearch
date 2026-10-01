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
else:
    raise SystemExit(f"unknown mode: {mode}")
