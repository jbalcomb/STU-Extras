#!/usr/bin/env python3
"""
Generate an IDC script that renames MAGIC functions to WIZARDS names.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Dict, List, Set, Tuple


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate MAGIC rename IDC from ida55-function-match.json.")
    parser.add_argument(
        "--match",
        default="out/ida55-function-match.json",
        help="Machine-code match JSON produced by ida55_match_functions.py",
    )
    parser.add_argument(
        "--output",
        default=str(Path("out") / "ida55-sync" / "magic_rename_to_wizards.idc"),
        help="Output IDC script path",
    )
    parser.add_argument(
        "--manifest",
        default=str(Path("out") / "ida55-sync" / "magic_rename_to_wizards.json"),
        help="Output manifest JSON path",
    )
    return parser.parse_args()


def escape_idc_string(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def collect_position_renames(match_data: Dict[str, object]) -> List[Dict[str, object]]:
    renames: List[Dict[str, object]] = []
    seen: Set[Tuple[str, str, str]] = set()
    for item in match_data.get("position_override_pairs", []):
        magic = item["magic"]
        wizards = item["wizards"]
        old_name = str(magic["name"])
        new_name = str(wizards["name"])
        if not item.get("effective_exact_match"):
            continue
        if old_name == new_name:
            continue
        key = (str(magic["segment"]), str(magic["start_ea"]), old_name)
        if key in seen:
            continue
        seen.add(key)
        renames.append(
            {
                "old_name": old_name,
                "new_name": new_name,
                "magic_segment": str(magic["segment"]),
                "magic_start_ea": str(magic["start_ea"]),
                "wizards_segment": str(wizards["segment"]),
                "wizards_start_ea": str(wizards["start_ea"]),
                "category": str(item.get("category")),
                "override_segment_pair": item.get("override_segment_pair"),
            }
        )
    renames.sort(key=lambda item: (item["magic_segment"], item["magic_start_ea"], item["old_name"]))
    return renames


def render_idc(renames: List[Dict[str, object]]) -> str:
    lines: List[str] = [
        "#include <idc.idc>",
        "",
        "static rename_magic_function(old_name, new_name)",
        "{",
        "  auto ea;",
        "  ea = LocByName(old_name);",
        "  if (ea == BADADDR) {",
        '    Message("Missing MAGIC symbol: %s\\n", old_name);',
        "    return;",
        "  }",
        "  if (Name(ea) == new_name) {",
        "    return;",
        "  }",
        "  if (!MakeNameEx(ea, new_name, SN_CHECK|SN_NOWARN)) {",
        '    Message("Rename failed: %s -> %s\\n", old_name, new_name);',
        "  }",
        "}",
        "",
        "static main()",
        "{",
    ]
    for item in renames:
        lines.append(
            '  rename_magic_function("%s", "%s");'
            % (escape_idc_string(item["old_name"]), escape_idc_string(item["new_name"]))
        )
    lines.extend(["}", ""])
    return "\n".join(lines)


def main() -> int:
    args = parse_args()
    match_data = json.loads(Path(args.match).read_text(encoding="utf-8"))
    renames = collect_position_renames(match_data)

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(render_idc(renames), encoding="utf-8")

    manifest_path = Path(args.manifest)
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps({"rename_count": len(renames), "renames": renames}, indent=2), encoding="utf-8")

    print(f"Wrote {output_path}")
    print(f"Wrote {manifest_path}")
    print(f"Rename count: {len(renames)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
