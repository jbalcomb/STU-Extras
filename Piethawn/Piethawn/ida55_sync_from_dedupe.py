#!/usr/bin/env python3
"""
Generate IDC scripts that annotate equivalent procedures in IDA 5.5.

The generated IDC scripts use function names, not addresses, so they are safe
to apply to independently rebuilt IDBs as long as the current symbol names
still match the split ASM export.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Dict, List


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate IDC sync scripts from asm_dedupe_index output.")
    parser.add_argument(
        "--index",
        default="out/asm-dedupe-index.json",
        help="Duplicate index JSON produced by asm_dedupe_index.py",
    )
    parser.add_argument(
        "--output-dir",
        default="out/ida55-sync",
        help="Directory where IDC scripts and manifests should be written.",
    )
    return parser.parse_args()


def escape_idc_string(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def canonical_mappings(index_data: Dict[str, object]) -> List[Dict[str, object]]:
    results: List[Dict[str, object]] = []
    for bucket_name in ("exact_raw_groups", "normalized_groups", "mnemonic_groups"):
        for group in index_data.get(bucket_name, []):
            canonical = group["canonical"]
            for member in group["members"]:
                if member["relative_path"] == canonical["relative_path"]:
                    continue
                if canonical["proc_name"] != member["proc_name"]:
                    continue
                results.append(
                    {
                        "source_bucket": bucket_name,
                        "canonical": canonical,
                        "duplicate": member,
                        "same_name": canonical["proc_name"] == member["proc_name"],
                    }
                )
    return results


def render_idc(game: str, mappings: List[Dict[str, object]]) -> str:
    lines: List[str] = [
        '#include <idc.idc>',
        '',
        'static annotate_equivalent(old_name, canonical_name, canonical_path, canonical_game, bucket)',
        '{',
        '  auto ea;',
        '  auto cmt;',
        '  ea = LocByName(old_name);',
        '  if (ea == BADADDR) {',
        '    Message("Missing symbol: %s\\n", old_name);',
        '    return;',
        '  }',
        '  cmt = form("Equivalent to %s in %s (%s) [%s]", canonical_name, canonical_game, canonical_path, bucket);',
        '  SetFunctionCmt(ea, cmt, 1);',
        '}',
        '',
        'static main()',
        '{',
    ]

    for mapping in mappings:
        duplicate = mapping["duplicate"]
        canonical = mapping["canonical"]
        lines.append(
            '  annotate_equivalent("%s", "%s", "%s", "%s", "%s");'
            % (
                escape_idc_string(duplicate["proc_name"]),
                escape_idc_string(canonical["proc_name"]),
                escape_idc_string(canonical["relative_path"]),
                escape_idc_string(canonical["game"]),
                escape_idc_string(mapping["source_bucket"]),
            )
        )
    lines.extend(['}', ''])
    return "\n".join(lines)


def main() -> int:
    args = parse_args()
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    index_data = json.loads(Path(args.index).read_text(encoding="utf-8"))
    mappings = canonical_mappings(index_data)

    by_game: Dict[str, List[Dict[str, object]]] = {"MAGIC": [], "WIZARDS": []}
    for mapping in mappings:
        by_game[mapping["duplicate"]["game"]].append(mapping)

    for game, items in by_game.items():
        script_path = output_dir / f"{game.lower()}_equivalents.idc"
        script_path.write_text(render_idc(game, items), encoding="utf-8")

    (output_dir / "manifest.json").write_text(json.dumps({"mapping_count": len(mappings), "mappings": mappings}, indent=2), encoding="utf-8")
    print(f"Wrote {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
