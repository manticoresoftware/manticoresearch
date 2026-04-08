#!/usr/bin/env python3

import argparse
import collections
import importlib
import pathlib
import struct
import subprocess
import sys
import tempfile
from collections import OrderedDict

MAGIC = b"MSUKLEM1"
VERSION = 3
DEFAULT_PYMORPHY2_VERSION = "0.9.1"
DEFAULT_PYMORPHY2_DICTS_UK_VERSION = "2.4.1.1.1460299261"
SECTION_STRINGS = 1
SECTION_EXACT = 2
SECTION_PREDICTIONS = 3
CODEC_NONE = 0
CODEC_LZ4 = 1

def pack_string(value: str) -> bytes:
    encoded = value.encode("utf-8")
    return struct.pack("<I", len(encoded)) + encoded


def compress_section(payload: bytes) -> tuple[int, bytes]:
    lz4_block = importlib.import_module("lz4.block")
    compressed = lz4_block.compress(payload, store_size=False)
    if len(compressed) >= len(payload):
        return CODEC_NONE, payload
    return CODEC_LZ4, compressed


def build_string_table(
    entries: OrderedDict[str, list[str]],
    predictions: list[tuple[str, str, str, str, str, str, str, int]],
) -> tuple[list[str], dict[str, int]]:
    string_ids: OrderedDict[str, int] = OrderedDict()

    def add(value: str) -> None:
        if value not in string_ids:
            string_ids[value] = len(string_ids)

    for word, lemmas in entries.items():
        add(word)
        for lemma in lemmas:
            add(lemma)

    for ending, req_prefix, fixed_suffix, cur_prefix, cur_suffix, norm_prefix, norm_suffix, _count in predictions:
        for value in (ending, req_prefix, fixed_suffix, cur_prefix, cur_suffix, norm_prefix, norm_suffix):
            add(value)

    strings = list(string_ids.keys())
    return strings, dict(string_ids)


def write_asset_v3(
    entries: OrderedDict[str, list[str]],
    predictions: list[tuple[str, str, str, str, str, str, str, int]],
    min_word_length: int,
    max_suffix_length: int,
    out_path: str,
) -> None:
    strings, string_ids = build_string_table(entries, predictions)

    strings_payload = bytearray()
    strings_payload.extend(struct.pack("<I", len(strings)))
    for value in strings:
        strings_payload.extend(pack_string(value))

    exact_payload = bytearray()
    exact_payload.extend(struct.pack("<I", len(entries)))
    for word, lemmas in entries.items():
        exact_payload.extend(struct.pack("<II", string_ids[word], len(lemmas)))
        for lemma in lemmas:
            exact_payload.extend(struct.pack("<I", string_ids[lemma]))

    prediction_payload = bytearray()
    prediction_payload.extend(struct.pack("<I", len(predictions)))
    for ending, req_prefix, fixed_suffix, cur_prefix, cur_suffix, norm_prefix, norm_suffix, count in predictions:
        prediction_payload.extend(
            struct.pack(
                "<IIIIIIII",
                string_ids[ending],
                string_ids[req_prefix],
                string_ids[fixed_suffix],
                string_ids[cur_prefix],
                string_ids[cur_suffix],
                string_ids[norm_prefix],
                string_ids[norm_suffix],
                count,
            )
        )

    section_payloads = [
        (SECTION_STRINGS, bytes(strings_payload)),
        (SECTION_EXACT, bytes(exact_payload)),
        (SECTION_PREDICTIONS, bytes(prediction_payload)),
    ]

    compressed_sections: list[tuple[int, int, bytes, int]] = []
    for section_type, payload in section_payloads:
        codec, packed = compress_section(payload)
        compressed_sections.append((section_type, codec, packed, len(payload)))

    header = bytearray()
    header.extend(MAGIC)
    header.extend(struct.pack("<I", VERSION))
    header.extend(struct.pack("<I", len(compressed_sections)))
    header.extend(struct.pack("<I", min_word_length))
    header.extend(struct.pack("<I", max_suffix_length))
    header.extend(struct.pack("<I", 0))

    section_table = bytearray()
    offset = len(header) + len(compressed_sections) * struct.calcsize("<IIQQQII")
    for section_type, codec, packed, unpacked_size in compressed_sections:
        section_table.extend(
            struct.pack(
                "<IIQQQII",
                section_type,
                codec,
                offset,
                len(packed),
                unpacked_size,
                0,
                0,
            )
        )
        offset += len(packed)

    with open(out_path, "wb") as dst:
        dst.write(header)
        dst.write(section_table)
        for _section_type, _codec, packed, _unpacked_size in compressed_sections:
            dst.write(packed)


def dedupe_entries(mapping: dict[str, list[str]]) -> OrderedDict[str, list[str]]:
    entries: OrderedDict[str, list[str]] = OrderedDict()
    for word in sorted(mapping.keys()):
        seen = set()
        lemmas = []
        for lemma in mapping[word]:
            if lemma not in seen:
                seen.add(lemma)
                lemmas.append(lemma)
        if lemmas:
            entries[word] = lemmas
    return entries


def convert_pymorphy2_dictionary(dict_path: str) -> OrderedDict[str, list[str]]:
    wrapper = importlib.import_module("pymorphy2.opencorpora_dict.wrapper")
    dictionary = wrapper.Dictionary(dict_path)

    mapping: dict[str, list[str]] = collections.defaultdict(list)
    for word, _tag, normal_form, _para_id, _idx in dictionary.iter_known_words():
        mapping[word].append(normal_form)

    return dedupe_entries(mapping)


def extract_prediction_rules(dict_path: str) -> tuple[list[tuple[str, str, str, str, str, str, str, int]], int, int]:
    wrapper = importlib.import_module("pymorphy2.opencorpora_dict.wrapper")
    dictionary = wrapper.Dictionary(dict_path)
    compile_options = dictionary.meta.get("compile_options", {})
    min_word_length = 4
    max_suffix_length = int(compile_options.get("max_suffix_length", 5))

    predictions: list[tuple[str, str, str, str, str, str, str, int]] = []
    for prefix_id, prediction_dawg in enumerate(dictionary.prediction_suffixes_dawgs):
        required_prefix = dictionary.paradigm_prefixes[prefix_id]
        for fixed_suffix, payload in prediction_dawg.iteritems():
            count, para_id, idx = payload
            paradigm = dictionary.paradigms[para_id]
            paradigm_len = len(paradigm) // 3

            cur_suffix = dictionary.suffixes[paradigm[idx]]
            cur_prefix = dictionary.paradigm_prefixes[paradigm[paradigm_len * 2 + idx]]
            norm_suffix = dictionary.suffixes[paradigm[0]]
            norm_prefix = dictionary.paradigm_prefixes[paradigm[paradigm_len * 2]]

            predictions.append(
                (
                    fixed_suffix,
                    required_prefix,
                    fixed_suffix,
                    cur_prefix,
                    cur_suffix,
                    norm_prefix,
                    norm_suffix,
                    int(count),
                )
            )

    predictions.sort(key=lambda item: (-item[7], len(item[0]), item[0], item[1], item[2], item[3], item[4], item[5], item[6]))
    return predictions, min_word_length, max_suffix_length


def bootstrap_pymorphy2_target(target_dir: str, pymorphy2_version: str, dicts_uk_version: str) -> None:
    cmd = [
        sys.executable,
        "-m",
        "pip",
        "install",
        "--disable-pip-version-check",
        "--no-deps",
        "--target",
        target_dir,
        f"pymorphy2=={pymorphy2_version}",
        f"pymorphy2-dicts-uk=={dicts_uk_version}",
        "dawg-python>=0.7.1",
        "lz4>=4",
    ]
    subprocess.check_call(cmd)


def build_from_upstream(output_pak: str, pymorphy2_version: str, dicts_uk_version: str) -> None:
    with tempfile.TemporaryDirectory(prefix="uklemmatizer-") as tmpdir:
        bootstrap_pymorphy2_target(tmpdir, pymorphy2_version, dicts_uk_version)
        sys.path.insert(0, tmpdir)
        try:
            pymorphy2_dicts_uk = importlib.import_module("pymorphy2_dicts_uk")
            dict_path = pymorphy2_dicts_uk.get_path()
            entries = convert_pymorphy2_dictionary(dict_path)
            predictions, min_word_length, max_suffix_length = extract_prediction_rules(dict_path)
            write_asset_v3(entries, predictions, min_word_length, max_suffix_length, output_pak)
        finally:
            sys.path.remove(tmpdir)


def build_from_dict_dir(dict_dir: str, output_pak: str) -> None:
    entries = convert_pymorphy2_dictionary(dict_dir)
    predictions, min_word_length, max_suffix_length = extract_prediction_rules(dict_dir)
    write_asset_v3(entries, predictions, min_word_length, max_suffix_length, output_pak)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build a native Ukrainian lemmatizer asset."
    )
    parser.add_argument("output_pak", help="Output native asset path, usually uk.pak")
    parser.add_argument(
        "--from-upstream",
        action="store_true",
        help="Bootstrap pymorphy2 + pymorphy2-dicts-uk in a temp target and convert the upstream compiled dictionary.",
    )
    parser.add_argument(
        "--dict-dir",
        help="Convert an already available pymorphy2 Ukrainian dictionary directory.",
    )
    parser.add_argument("--pymorphy2-version", default=DEFAULT_PYMORPHY2_VERSION)
    parser.add_argument("--pymorphy2-dicts-uk-version", default=DEFAULT_PYMORPHY2_DICTS_UK_VERSION)
    return parser.parse_args()


def main() -> None:
    args = parse_args()

    output_dir = pathlib.Path(args.output_pak).resolve().parent
    output_dir.mkdir(parents=True, exist_ok=True)

    if args.from_upstream:
        if args.dict_dir:
            raise ValueError("--dict-dir cannot be used with --from-upstream")
        build_from_upstream(args.output_pak, args.pymorphy2_version, args.pymorphy2_dicts_uk_version)
        return

    if args.dict_dir:
        build_from_dict_dir(args.dict_dir, args.output_pak)
        return

    raise ValueError("use --from-upstream, or use --dict-dir")


if __name__ == "__main__":
    main()
