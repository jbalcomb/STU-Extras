#!/usr/bin/env python3
"""Generate IDA IDC scripts from a completion-status sync plan."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Dict, List, Optional, Sequence


DEFAULT_PLAN = Path("ida_proc_status_sync") / "ida_proc_status_sync.plan.json"
DEFAULT_MAGIC_IDC = Path("ida_proc_status_sync") / "mgc_proc_status_sync.idc"
DEFAULT_WIZARDS_IDC = Path("ida_proc_status_sync") / "wzd_proc_status_sync.idc"


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate completion-status IDC scripts from a status sync plan.")
    parser.add_argument("--plan", default=str(DEFAULT_PLAN))
    parser.add_argument("--magic-idc", default=str(DEFAULT_MAGIC_IDC))
    parser.add_argument("--wizards-idc", default=str(DEFAULT_WIZARDS_IDC))
    return parser.parse_args(argv)


def normalize_hex_color(value: str) -> str:
    text = value.strip().upper()
    if text.startswith("0X"):
        text = text[2:]
    return text


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


def write_status_idc(plan: Dict[str, object], magic_idc: Path, wizards_idc: Path) -> None:
    summary = plan.get("summary", {})
    if not isinstance(summary, dict):
        raise ValueError("Plan summary must be an object.")
    updates_by_game = plan.get("updates_by_game", {})
    if not isinstance(updates_by_game, dict):
        raise ValueError("Plan updates_by_game must be an object.")
    completed_color_hex = str(summary.get("completed_color_hex", ""))

    magic_updates = updates_by_game.get("MAGIC", [])
    wizards_updates = updates_by_game.get("WIZARDS", [])
    if not isinstance(magic_updates, list) or not isinstance(wizards_updates, list):
        raise ValueError("Plan updates for MAGIC and WIZARDS must be lists.")

    magic_idc.parent.mkdir(parents=True, exist_ok=True)
    wizards_idc.parent.mkdir(parents=True, exist_ok=True)
    magic_idc.write_text(render_color_sync_idc("MAGIC", magic_updates, completed_color_hex, "completion-sync.log"), encoding="utf-8")
    wizards_idc.write_text(render_color_sync_idc("WIZARDS", wizards_updates, completed_color_hex, "completion-sync.log"), encoding="utf-8")


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    plan_path = Path(args.plan)
    plan = json.loads(plan_path.read_text(encoding="utf-8"))
    write_status_idc(plan, Path(args.magic_idc), Path(args.wizards_idc))
    print(f"Wrote {args.magic_idc}")
    print(f"Wrote {args.wizards_idc}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
