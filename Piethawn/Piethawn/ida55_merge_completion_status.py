#!/usr/bin/env python3
"""
Merge function completion state across duplicate/equivalent functions and
generate IDC scripts to sync completion colors in either IDA database.
"""

from __future__ import annotations

import argparse
import json
from collections import Counter, defaultdict
from pathlib import Path
from typing import DefaultDict, Dict, Iterable, List, Set, Tuple


NodeKey = Tuple[str, str, str, str]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Merge function completion colors across MAGIC and WIZARDS.")
    parser.add_argument("--magic-colors", default=str(Path("in") / "MAGIC.idb.ida55-function-colors.json"))
    parser.add_argument("--wizards-colors", default=str(Path("in") / "WIZARDS.idb.ida55-function-colors.json"))
    parser.add_argument("--match", default=str(Path("out") / "ida55-function-match.json"))
    parser.add_argument(
        "--completed-color-hex",
        default="00FFFF80",
        help="IDA function color to treat as completed (0xBBGGRR, without 0x prefix). Default: 00FFFF80",
    )
    parser.add_argument("--output", default=str(Path("out") / "ida55-sync" / "completion-merge.json"))
    parser.add_argument("--magic-idc", default=str(Path("out") / "ida55-sync" / "magic_sync_completion.idc"))
    parser.add_argument("--wizards-idc", default=str(Path("out") / "ida55-sync" / "wizards_sync_completion.idc"))
    return parser.parse_args()


def normalize_hex_color(value: str) -> str:
    text = value.strip().upper()
    if text.startswith("0X"):
        text = text[2:]
    return text


def node_key(game: str, item: Dict[str, object]) -> NodeKey:
    return (game, str(item.get("segment")), str(item.get("start_ea")), str(item.get("name")))


def load_color_dump(path: Path, game: str) -> Tuple[Dict[NodeKey, Dict[str, object]], Dict[str, object]]:
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


def add_edge(graph: DefaultDict[NodeKey, Set[NodeKey]], left: NodeKey, right: NodeKey) -> None:
    if left == right:
        return
    graph[left].add(right)
    graph[right].add(left)


def build_graph(match_data: Dict[str, object]) -> DefaultDict[NodeKey, Set[NodeKey]]:
    graph: DefaultDict[NodeKey, Set[NodeKey]] = defaultdict(set)

    for group in match_data.get("exact_byte_groups", []):
        members = [node_key(str(item["game"]), item) for item in group.get("members", [])]
        for idx, left in enumerate(members):
            graph[left]
            for right in members[idx + 1 :]:
                add_edge(graph, left, right)

    for item in match_data.get("position_override_pairs", []):
        left = node_key("MAGIC", item["magic"])
        right = node_key("WIZARDS", item["wizards"])
        add_edge(graph, left, right)

    return graph


def connected_components(graph: DefaultDict[NodeKey, Set[NodeKey]], known_nodes: Iterable[NodeKey]) -> List[List[NodeKey]]:
    all_nodes: Set[NodeKey] = set(known_nodes)
    all_nodes.update(graph.keys())
    for neighbors in graph.values():
        all_nodes.update(neighbors)

    components: List[List[NodeKey]] = []
    seen: Set[NodeKey] = set()
    for start in sorted(all_nodes):
        if start in seen:
            continue
        stack = [start]
        component: List[NodeKey] = []
        while stack:
            node = stack.pop()
            if node in seen:
                continue
            seen.add(node)
            component.append(node)
            stack.extend(sorted(graph.get(node, ()), reverse=True))
        components.append(sorted(component))
    return components


def idc_color_literal(color_hex: str) -> str:
    return f"0x{normalize_hex_color(color_hex)}"


def escape_idc_string(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def render_color_sync_idc(game: str, updates: List[Dict[str, object]], completed_color_hex: str, log_name: str) -> str:
    lines: List[str] = [
        "#include <idc.idc>",
        "",
        "static log_line(handle, message)",
        "{",
        '  fprintf(handle, "%s\\n", message);',
        "}",
        "",
        "static sync_completion(log_handle, old_name, desired_color)",
        "{",
        "  auto ea;",
        "  auto current_color;",
        "  ea = LocByName(old_name);",
        "  if (ea == BADADDR) {",
        '    Message("Missing symbol: %s\\n", old_name);',
        '    log_line(log_handle, form("missing,%s", old_name));',
        "    return;",
        "  }",
        "  current_color = GetColor(ea, CIC_FUNC);",
        "  if (current_color == desired_color) {",
        '    log_line(log_handle, form("already-set,%s,%08X", old_name, current_color));',
        "    return;",
        "  }",
        "  if (!SetColor(ea, CIC_FUNC, desired_color)) {",
        '    Message("SetColor failed: %s\\n", old_name);',
        '    log_line(log_handle, form("setcolor-failed,%s,%08X,%08X", old_name, current_color, desired_color));',
        "    return;",
        "  }",
        '  log_line(log_handle, form("updated,%s,%08X,%08X", old_name, current_color, desired_color));',
        "}",
        "",
        "static main()",
        "{",
        "  auto log_path;",
        "  auto log_handle;",
        "  auto update_count;",
        f"  update_count = {len(updates)};",
        f'  log_path = GetIdbPath() + ".{escape_idc_string(log_name)}";',
        '  log_handle = fopen(log_path, "w");',
        "  if (log_handle == 0) {",
        '    Warning("Could not open log file: %s", log_path);',
        "    return;",
        "  }",
        f"  Message(\"Applying completion color {escape_idc_string(completed_color_hex)} to {game}...\\n\");",
        '  log_line(log_handle, form("game,%s", "' + escape_idc_string(game) + '"));',
        '  log_line(log_handle, form("completed_color,%s", "' + escape_idc_string(completed_color_hex) + '"));',
        '  log_line(log_handle, form("update_count,%d", update_count));',
    ]
    for item in updates:
        lines.append(
            '  sync_completion(log_handle, "%s", %s);'
            % (escape_idc_string(str(item["name"])), idc_color_literal(str(item["desired_color_hex"])))
        )
    lines.extend(
        [
            "  fclose(log_handle);",
            '  Message("Wrote completion sync log to %s\\n", log_path);',
            "}",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    args = parse_args()
    completed_color_hex = normalize_hex_color(args.completed_color_hex)

    magic_nodes, magic_meta = load_color_dump(Path(args.magic_colors), "MAGIC")
    wizards_nodes, wizards_meta = load_color_dump(Path(args.wizards_colors), "WIZARDS")
    all_nodes = {**magic_nodes, **wizards_nodes}
    match_data = json.loads(Path(args.match).read_text(encoding="utf-8"))
    graph = build_graph(match_data)
    components = connected_components(graph, all_nodes.keys())

    component_payload: List[Dict[str, object]] = []
    updates_by_game: Dict[str, List[Dict[str, object]]] = {"MAGIC": [], "WIZARDS": []}
    exact_completed_counts = Counter()

    for component_index, component in enumerate(components):
        members = [all_nodes[node] for node in component if node in all_nodes]
        if not members:
            continue
        color_counts = Counter(str(item["item_color_hex"]) for item in members)
        completed_members = [item for item in members if str(item["item_color_hex"]) == completed_color_hex]
        is_completed = bool(completed_members)
        for item in completed_members:
            exact_completed_counts[item["game"]] += 1

        component_record = {
            "component_id": component_index,
            "member_count": len(members),
            "games": sorted({item["game"] for item in members}),
            "segments": sorted({item["segment"] for item in members}),
            "is_completed": is_completed,
            "completed_color_hex": completed_color_hex if is_completed else None,
            "completed_member_count": len(completed_members),
            "colors_present": dict(sorted(color_counts.items())),
            "members": members,
        }
        component_payload.append(component_record)

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
        updates_by_game[game].sort(key=lambda item: (item["segment"], item["start_ea"], item["name"]))

    payload = {
        "summary": {
            "magic_function_count": len(magic_nodes),
            "wizards_function_count": len(wizards_nodes),
            "component_count": len(component_payload),
            "completed_component_count": sum(1 for item in component_payload if item["is_completed"]),
            "completed_function_count_by_game": dict(sorted(exact_completed_counts.items())),
            "sync_update_count_by_game": {game: len(items) for game, items in sorted(updates_by_game.items())},
            "completed_color_hex": completed_color_hex,
        },
        "magic_meta": magic_meta,
        "wizards_meta": wizards_meta,
        "components": component_payload,
        "updates_by_game": updates_by_game,
    }

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")

    magic_idc_path = Path(args.magic_idc)
    magic_idc_path.parent.mkdir(parents=True, exist_ok=True)
    magic_idc_path.write_text(
        render_color_sync_idc("MAGIC", updates_by_game["MAGIC"], completed_color_hex, "completion-sync.log"),
        encoding="utf-8",
    )

    wizards_idc_path = Path(args.wizards_idc)
    wizards_idc_path.parent.mkdir(parents=True, exist_ok=True)
    wizards_idc_path.write_text(
        render_color_sync_idc("WIZARDS", updates_by_game["WIZARDS"], completed_color_hex, "completion-sync.log"),
        encoding="utf-8",
    )

    print(f"Wrote {output_path}")
    print(f"Wrote {magic_idc_path}")
    print(f"Wrote {wizards_idc_path}")
    print(f"Completion color: {completed_color_hex}")
    print(f"MAGIC updates: {len(updates_by_game['MAGIC'])}")
    print(f"WIZARDS updates: {len(updates_by_game['WIZARDS'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
