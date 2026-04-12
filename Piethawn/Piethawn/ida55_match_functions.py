#!/usr/bin/env python3
"""
Compare IDA 5.5 function dumps using machine-code hashes first.
"""

from __future__ import annotations

import argparse
import json
from collections import defaultdict
from pathlib import Path
from typing import DefaultDict, Dict, List, Sequence, Set, Tuple


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Match IDA 5.5 function dumps by machine-code hashes.")
    parser.add_argument("--magic", default="MAGIC.idb.ida55-functions.json", help="MAGIC function dump JSON")
    parser.add_argument("--wizards", default="WIZARDS.idb.ida55-functions.json", help="WIZARDS function dump JSON")
    parser.add_argument(
        "--overrides",
        default="ida55_exact_match_overrides.json",
        help="JSON file containing known-exact segment pair overrides",
    )
    parser.add_argument("--output", default="out/ida55-function-match.json", help="Output JSON path")
    return parser.parse_args()


def simplify_function(game: str, item: Dict[str, object]) -> Dict[str, object]:
    return {
        "game": game,
        "name": item.get("name"),
        "segment": item.get("segment"),
        "start_ea": item.get("start_ea"),
        "end_ea": item.get("end_ea"),
        "size": item.get("size"),
        "chunk_count": item.get("chunk_count"),
        "bytes_sha256": item.get("bytes_sha256"),
        "chunk_bytes_sha256": item.get("chunk_bytes_sha256", []),
    }


def load_dump(path: Path, game: str) -> List[Dict[str, object]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    return [simplify_function(game, item) for item in data.get("functions", []) if item.get("name")]


def load_overrides(path: Path) -> List[Dict[str, str]]:
    if not path.exists():
        return []
    data = json.loads(path.read_text(encoding="utf-8"))
    segment_pairs: List[Dict[str, str]] = []
    for item in data.get("segment_pairs", []):
        segment_pairs.append(
            {
            "magic_segment": str(item["magic_segment"]),
            "wizards_segment": str(item["wizards_segment"]),
            "pairing": str(item.get("pairing", "by_position")),
            }
        )
    return segment_pairs


def ea_sort_key(value: object) -> int:
    text = str(value or "")
    if ":" not in text:
        return 0
    try:
        return int(text.split(":", 1)[1], 16)
    except ValueError:
        return 0


def function_identity(item: Dict[str, object]) -> Tuple[str, str, str]:
    return (str(item["segment"]), str(item["name"]), str(item["start_ea"]))


def pair_override_segments_by_position(
    magic_functions: Sequence[Dict[str, object]],
    wizards_functions: Sequence[Dict[str, object]],
    overrides: Sequence[Dict[str, str]],
) -> List[Dict[str, object]]:
    magic_by_segment: DefaultDict[str, List[Dict[str, object]]] = defaultdict(list)
    wizards_by_segment: DefaultDict[str, List[Dict[str, object]]] = defaultdict(list)
    for item in magic_functions:
        magic_by_segment[str(item["segment"])].append(item)
    for item in wizards_functions:
        wizards_by_segment[str(item["segment"])].append(item)

    pairs: List[Dict[str, object]] = []
    for override in overrides:
        if override.get("pairing") != "by_position":
            continue
        magic_items = sorted(magic_by_segment.get(override["magic_segment"], []), key=lambda item: ea_sort_key(item["start_ea"]))
        wizards_items = sorted(wizards_by_segment.get(override["wizards_segment"], []), key=lambda item: ea_sort_key(item["start_ea"]))
        pair_count = min(len(magic_items), len(wizards_items))
        for index in range(pair_count):
            magic_item = magic_items[index]
            wizards_item = wizards_items[index]
            exact = magic_item["bytes_sha256"] == wizards_item["bytes_sha256"]
            same_size = magic_item.get("size") == wizards_item.get("size")
            same_chunks = magic_item.get("chunk_bytes_sha256") == wizards_item.get("chunk_bytes_sha256")
            same_name = magic_item.get("name") == wizards_item.get("name")
            category = "exact_bytes" if exact else "exact_override_segment_position"
            pairs.append(
                {
                    "pairing": "by_position",
                    "position_index": index,
                    "magic": magic_item,
                    "wizards": wizards_item,
                    "category": category,
                    "effective_exact_match": True,
                    "exact_bytes": exact,
                    "same_name": same_name,
                    "same_size": same_size,
                    "same_chunk_hashes": same_chunks,
                    "override_segment_pair": override,
                }
            )
    return pairs


def group_exact_byte_matches(all_functions: Sequence[Dict[str, object]]) -> List[Dict[str, object]]:
    buckets: DefaultDict[str, List[Dict[str, object]]] = defaultdict(list)
    for item in all_functions:
        digest = item.get("bytes_sha256")
        if not digest:
            continue
        buckets[str(digest)].append(item)

    groups: List[Dict[str, object]] = []
    for digest, items in sorted(buckets.items()):
        games = {item["game"] for item in items}
        if len(items) < 2 or len(games) < 2:
            continue
        groups.append(
            {
                "bytes_sha256": digest,
                "members": sorted(items, key=lambda item: (item["game"], item["segment"], item["name"])),
            }
        )
    return groups


def pair_same_name(
    magic_functions: Sequence[Dict[str, object]],
    wizards_functions: Sequence[Dict[str, object]],
    positional_override_pairs: Sequence[Dict[str, object]],
) -> List[Dict[str, object]]:
    magic_by_name: DefaultDict[str, List[Dict[str, object]]] = defaultdict(list)
    wizards_by_name: DefaultDict[str, List[Dict[str, object]]] = defaultdict(list)
    for item in magic_functions:
        magic_by_name[str(item["name"])].append(item)
    for item in wizards_functions:
        wizards_by_name[str(item["name"])].append(item)
    positional_identities: Set[Tuple[Tuple[str, str, str], Tuple[str, str, str]]] = {
        (function_identity(item["magic"]), function_identity(item["wizards"])) for item in positional_override_pairs
    }

    pairs: List[Dict[str, object]] = []
    for name in sorted(set(magic_by_name) & set(wizards_by_name)):
        for magic_item in magic_by_name[name]:
            for wizards_item in wizards_by_name[name]:
                exact = magic_item["bytes_sha256"] == wizards_item["bytes_sha256"]
                same_size = magic_item.get("size") == wizards_item.get("size")
                same_chunks = magic_item.get("chunk_bytes_sha256") == wizards_item.get("chunk_bytes_sha256")
                identity_pair = (function_identity(magic_item), function_identity(wizards_item))
                positional_override = identity_pair in positional_identities
                effective_exact = exact or positional_override
                if exact:
                    category = "exact_bytes"
                elif positional_override:
                    category = "exact_override_segment_position"
                elif same_chunks:
                    category = "same_chunk_hashes"
                elif same_size:
                    category = "same_name_same_size"
                else:
                    category = "same_name_different_bytes"
                pairs.append(
                    {
                        "name": name,
                        "magic": magic_item,
                        "wizards": wizards_item,
                        "category": category,
                        "effective_exact_match": effective_exact,
                        "exact_bytes": exact,
                        "same_size": same_size,
                        "same_chunk_hashes": same_chunks,
                        "override_segment_pair": {
                            "magic_segment": str(magic_item["segment"]),
                            "wizards_segment": str(wizards_item["segment"]),
                            "pairing": "by_position",
                        }
                        if positional_override
                        else None,
                    }
                )
    return pairs


def main() -> int:
    args = parse_args()
    magic_functions = load_dump(Path(args.magic), "MAGIC")
    wizards_functions = load_dump(Path(args.wizards), "WIZARDS")
    all_functions = magic_functions + wizards_functions
    overrides = load_overrides(Path(args.overrides))
    positional_override_pairs = pair_override_segments_by_position(magic_functions, wizards_functions, overrides)

    payload = {
        "summary": {
            "magic_function_count": len(magic_functions),
            "wizards_function_count": len(wizards_functions),
            "cross_game_exact_byte_groups": 0,
            "position_override_pair_count": 0,
            "position_override_name_mismatch_count": 0,
            "same_name_pair_count": 0,
            "effective_exact_pair_count": 0,
            "override_pair_count": 0,
        },
        "exact_match_overrides": sorted(overrides, key=lambda item: (item["magic_segment"], item["wizards_segment"])),
        "exact_byte_groups": group_exact_byte_matches(all_functions),
        "position_override_pairs": positional_override_pairs,
        "same_name_pairs": pair_same_name(magic_functions, wizards_functions, positional_override_pairs),
    }
    payload["summary"]["cross_game_exact_byte_groups"] = len(payload["exact_byte_groups"])
    payload["summary"]["position_override_pair_count"] = len(payload["position_override_pairs"])
    payload["summary"]["position_override_name_mismatch_count"] = sum(
        1 for item in payload["position_override_pairs"] if not item["same_name"]
    )
    payload["summary"]["same_name_pair_count"] = len(payload["same_name_pairs"])
    payload["summary"]["effective_exact_pair_count"] = sum(
        1 for item in payload["same_name_pairs"] if item["effective_exact_match"]
    )
    payload["summary"]["override_pair_count"] = sum(
        1 for item in payload["position_override_pairs"] if not item["exact_bytes"]
    )

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(f"Wrote {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
