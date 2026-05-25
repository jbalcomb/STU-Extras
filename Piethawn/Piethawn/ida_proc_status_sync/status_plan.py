#!/usr/bin/env python3
"""Build a function completion-status sync plan from current function names and colors."""

from __future__ import annotations

import argparse
import json
from collections import Counter, defaultdict
from pathlib import Path
from typing import DefaultDict, Dict, List, Optional, Sequence, Tuple


NodeKey = Tuple[str, str]
DEFAULT_COMPLETED_COLOR_HEX = "00FFFF80"
DEFAULT_OUTPUT = Path("ida_proc_status_sync") / "ida_proc_status_sync.plan.json"


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build a completion-status sync plan from current function names and colors.")
    parser.add_argument("--magic-functions", default=str(Path("in") / "MAGIC.idb.ida55-functions.json"))
    parser.add_argument("--wizards-functions", default=str(Path("in") / "WIZARDS.idb.ida55-functions.json"))
    parser.add_argument(
        "--completed-color-hex",
        default=DEFAULT_COMPLETED_COLOR_HEX,
        help="IDA function color to treat as completed (0xBBGGRR, without 0x prefix). Default: 00FFFF80",
    )
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT))
    return parser.parse_args(argv)


def normalize_hex_color(value: str) -> str:
    text = value.strip().upper()
    if text.startswith("0X"):
        text = text[2:]
    return text


def node_key(game: str, item: Dict[str, object]) -> NodeKey:
    return (game, str(item.get("name")))


def load_function_dump(path: Path, game: str) -> Tuple[Dict[NodeKey, Dict[str, object]], Dict[str, object]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    nodes: Dict[NodeKey, Dict[str, object]] = {}
    for item in data.get("functions", []):
        key = node_key(game, item)
        nodes[key] = {
            "game": game,
            "name": str(item.get("name")),
            "segment": str(item.get("segment")),
            "entry_ea": str(item.get("entry_ea")),
            "start_ea": str(item.get("start_ea")),
            "end_ea": str(item.get("end_ea")),
            "size": item.get("size"),
            "item_color": item.get("item_color"),
            "item_color_hex": normalize_hex_color(str(item.get("item_color_hex", ""))),
            "funcattr_color": item.get("funcattr_color"),
            "funcattr_color_hex": normalize_hex_color(str(item.get("funcattr_color_hex", ""))),
            "has_user_color": bool(item.get("has_user_color")),
        }
    return nodes, data.get("meta", {})


def group_nodes_by_name(nodes: Dict[NodeKey, Dict[str, object]]) -> List[List[Dict[str, object]]]:
    groups: DefaultDict[str, List[Dict[str, object]]] = defaultdict(list)
    for item in nodes.values():
        groups[str(item["name"])].append(item)
    return [sorted(items, key=lambda item: (str(item["game"]), str(item["segment"]), str(item["start_ea"]))) for _, items in sorted(groups.items())]


def build_status_plan(
    magic_nodes: Dict[NodeKey, Dict[str, object]],
    wizards_nodes: Dict[NodeKey, Dict[str, object]],
    completed_color_hex: str,
    magic_meta: Optional[Dict[str, object]] = None,
    wizards_meta: Optional[Dict[str, object]] = None,
) -> Dict[str, object]:
    completed_color_hex = normalize_hex_color(completed_color_hex)
    all_nodes = {**magic_nodes, **wizards_nodes}

    component_payload: List[Dict[str, object]] = []
    updates_by_game: Dict[str, List[Dict[str, object]]] = {"MAGIC": [], "WIZARDS": []}
    exact_completed_counts = Counter()

    for component_index, members in enumerate(group_nodes_by_name(all_nodes)):
        color_counts = Counter(str(item["item_color_hex"]) for item in members)
        completed_members = [item for item in members if str(item["item_color_hex"]) == completed_color_hex]
        is_completed = bool(completed_members)
        for item in completed_members:
            exact_completed_counts[item["game"]] += 1

        component_payload.append(
            {
                "component_id": component_index,
                "name": str(members[0]["name"]),
                "member_count": len(members),
                "games": sorted({item["game"] for item in members}),
                "is_completed": is_completed,
                "completed_color_hex": completed_color_hex if is_completed else None,
                "completed_member_count": len(completed_members),
                "colors_present": dict(sorted(color_counts.items())),
                "members": members,
            }
        )

        if not is_completed:
            continue

        for item in members:
            if str(item["item_color_hex"]) == completed_color_hex:
                continue
            updates_by_game[item["game"]].append(
                {
                    "name": item["name"],
                    "segment": item["segment"],
                    "start_ea": item["start_ea"],
                    "current_color_hex": item["item_color_hex"],
                    "desired_color_hex": completed_color_hex,
                    "component_id": component_index,
                }
            )

    for game in updates_by_game:
        updates_by_game[game].sort(key=lambda item: (item["name"], item["segment"], item["start_ea"]))

    return {
        "summary": {
            "magic_function_count": len(magic_nodes),
            "wizards_function_count": len(wizards_nodes),
            "component_count": len(component_payload),
            "completed_component_count": sum(1 for item in component_payload if item["is_completed"]),
            "completed_function_count_by_game": dict(sorted(exact_completed_counts.items())),
            "sync_update_count_by_game": {game: len(items) for game, items in sorted(updates_by_game.items())},
            "completed_color_hex": completed_color_hex,
            "join_key": "name",
        },
        "magic_meta": magic_meta or {},
        "wizards_meta": wizards_meta or {},
        "components": component_payload,
        "updates_by_game": updates_by_game,
    }


def build_status_plan_from_files(
    magic_functions: Path,
    wizards_functions: Path,
    completed_color_hex: str,
) -> Dict[str, object]:
    magic_nodes, magic_meta = load_function_dump(magic_functions, "MAGIC")
    wizards_nodes, wizards_meta = load_function_dump(wizards_functions, "WIZARDS")
    return build_status_plan(magic_nodes, wizards_nodes, completed_color_hex, magic_meta, wizards_meta)


def write_json(path: Path, payload: Dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2), encoding="utf-8")


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    plan = build_status_plan_from_files(
        Path(args.magic_functions),
        Path(args.wizards_functions),
        args.completed_color_hex,
    )
    output_path = Path(args.output)
    write_json(output_path, plan)
    print(f"Wrote {output_path}")
    print(f"Completion color: {plan['summary']['completed_color_hex']}")
    print(f"Join key: {plan['summary']['join_key']}")
    print(f"MAGIC updates: {plan['summary']['sync_update_count_by_game']['MAGIC']}")
    print(f"WIZARDS updates: {plan['summary']['sync_update_count_by_game']['WIZARDS']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
