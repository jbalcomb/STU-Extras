#!/usr/bin/env python3
"""Generate an IDA IDC script from a minimal MAGIC procedure-name sync handoff."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any, Dict, Iterable, List


DEFAULT_RENAMES = Path("ida_proc_name_sync/magic_sync_names_from_wzd.json")
DEFAULT_OUTPUT = Path("ida_proc_name_sync/magic_sync_names_from_wzd.idc")


def load_json(path: Path) -> Dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def escape_idc_string(value: object) -> str:
    text = str(value)
    return text.replace("\\", "\\\\").replace('"', '\\"')


def load_renames(path: Path) -> List[Dict[str, str]]:
    payload = load_json(path)
    renames: List[Dict[str, str]] = []
    for index, rename in enumerate(payload.get("renames", []), 1):
        old_name = rename.get("old_name")
        new_name = rename.get("new_name")
        if not old_name or not new_name:
            raise ValueError(f"Rename entry {index} must have old_name and new_name.")
        renames.append({"old_name": str(old_name), "new_name": str(new_name)})
    return renames


def render_idc(renames: Iterable[Dict[str, Any]]) -> str:
    lines = [
        "#include <idc.idc>",
        "",
        "static sync_magic_proc_name(old_name, new_name)",
        "{",
        "    auto ea, current_name;",
        "",
        "    ea = LocByName(old_name);",
        "    if ( ea == BADADDR )",
        "    {",
        "        ea = LocByName(new_name);",
        "        if ( ea != BADADDR )",
        "        {",
        "            Message(\"already-synced,%s,%s,%a\\n\", old_name, new_name, ea);",
        "            return 1;",
        "        }",
        "        Message(\"not-found,%s,%s\\n\", old_name, new_name);",
        "        return 0;",
        "    }",
        "",
        "    current_name = GetFunctionName(ea);",
        "    if ( current_name != old_name )",
        "    {",
        "        Message(\"name-mismatch,%s,%s,%s,%a\\n\", old_name, new_name, current_name, ea);",
        "        return 0;",
        "    }",
        "",
        "    if ( LocByName(new_name) != BADADDR )",
        "    {",
        "        Message(\"collision,%s,%s\\n\", old_name, new_name);",
        "        return 0;",
        "    }",
        "",
        "    if ( !MakeNameEx(ea, new_name, SN_CHECK|SN_NOWARN) )",
        "    {",
        "        Message(\"rename-failed,%s,%s,%a\\n\", old_name, new_name, ea);",
        "        return 0;",
        "    }",
        "",
        "    Message(\"renamed,%s,%s,%a\\n\", old_name, new_name, ea);",
        "    return 1;",
        "}",
        "",
        "static main(void)",
        "{",
        "    auto ok, attempted;",
        "",
        "    ok = 0;",
        "    attempted = 0;",
        "    Batch(1);",
        "",
    ]
    for rename in renames:
        lines.append(
            '    attempted = attempted + 1; ok = ok + sync_magic_proc_name("%s", "%s");'
            % (escape_idc_string(rename["old_name"]), escape_idc_string(rename["new_name"]))
        )
    lines.extend(
        [
            "",
            "    Message(\"magic-name-sync-summary,attempted=%d,ok=%d,failed=%d\\n\", attempted, ok, attempted - ok);",
            "    Batch(0);",
            "    if ( ok == attempted )",
            "        Exit(0);",
            "    Exit(1);",
            "}",
            "",
        ]
    )
    return "\n".join(lines)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate MAGIC IDA IDC procedure-name sync script from minimal handoff JSON.")
    parser.add_argument("--renames", type=Path, default=DEFAULT_RENAMES, help="Minimal rename handoff JSON.")
    parser.add_argument("--output-idc", type=Path, default=DEFAULT_OUTPUT, help="Output IDC rename script.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    renames = load_renames(args.renames)
    args.output_idc.parent.mkdir(parents=True, exist_ok=True)
    args.output_idc.write_text(render_idc(renames), encoding="utf-8")
    print(f"Wrote {args.output_idc}")
    print(f"Rename count: {len(renames)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
