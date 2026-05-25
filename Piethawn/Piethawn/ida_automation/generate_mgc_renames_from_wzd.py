#!/usr/bin/env python3
"""Generate MGC procedure renames from WZD-owned duplicate procedure FWV rows."""

from __future__ import annotations

from pathlib import Path
from typing import Sequence, Tuple


DUPLICATE_PROCEDURES = Path("ida_automation/ida_duplicate_procedures_mgc_wzd.fwv")
DUPLICATE_SEGMENTS = Path("ida_automation/ida_duplicate_segments_mgc_wzd.fwv")
IGNORE_SEGMENTS = Path("ida_automation/ida_ignore_segments_mgc_wzd.fwv")
SPECIAL_SEGMENTS = Path("ida_automation/ida_special_segments_mgc_wzd.fwv")
OWNED_SEGMENTS = Path("ida_automation/ida_owned_segments_mgc_wzd.fwv")
RENAME_PLAN = Path("ida_automation/ida_rename_mgc_from_wzd.fwv")
RENAME_IDC = Path("ida_automation/ida_rename_mgc_from_wzd.idc")

SegmentPair = Tuple[str, str]
ProcedurePair = Tuple[str, str]
Rename = Tuple[str, str, str, str]


def read_pair_fwv(path: Path, header: tuple[str, str]) -> list[SegmentPair]:
    pairs: list[SegmentPair] = []
    for line_number, line in enumerate(path.read_text(encoding="utf-8-sig").splitlines(), 1):
        columns = line.split()
        if not columns:
            continue
        if line_number == 1 and tuple(columns) == header:
            continue
        if len(columns) != 2:
            raise ValueError(f"{path}:{line_number}: expected two columns")
        pairs.append((columns[0], columns[1]))
    return pairs


def read_owned_segments(path: Path = OWNED_SEGMENTS) -> set[tuple[str, str]]:
    owned: set[tuple[str, str]] = set()
    for line_number, line in enumerate(path.read_text(encoding="utf-8-sig").splitlines(), 1):
        columns = line.split()
        if not columns:
            continue
        if line_number == 1 and columns == ["OWNER", "SEGMENT"]:
            continue
        if len(columns) != 2:
            raise ValueError(f"{path}:{line_number}: expected OWNER and SEGMENT")
        owned.add((columns[0], columns[1]))
    return owned


def read_duplicate_procedures(path: Path = DUPLICATE_PROCEDURES) -> list[ProcedurePair]:
    pairs: list[ProcedurePair] = []
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        columns = line.split()
        if not columns:
            continue
        if line_number == 1 and columns == ["MAGIC", "WIZARDS"]:
            continue
        if len(columns) != 2:
            raise ValueError(f"{path}:{line_number}: expected two columns")
        pairs.append((columns[0], columns[1]))
    return pairs


def segment_token(procedure_id: str) -> str:
    return procedure_id.split(":", 1)[0].rsplit("p", 1)[0]


def ida_segment_name(token: str) -> str:
    if len(token) == 4 and token.startswith("s") and token[1:].isdigit():
        return "seg" + token[1:]
    return token


def procedure_name(procedure_id: str) -> str:
    return procedure_id.split(":", 1)[1]


def rename_plan() -> list[Rename]:
    ignore = set(read_pair_fwv(IGNORE_SEGMENTS, ("MAGIC", "WIZARDS")))
    special = set(read_pair_fwv(SPECIAL_SEGMENTS, ("MAGIC", "WIZARDS")))
    duplicate_segments = set(read_pair_fwv(DUPLICATE_SEGMENTS, ("MAGIC", "WIZARDS")))
    owned = read_owned_segments()
    renames: list[Rename] = []

    for magic_id, wizards_id in read_duplicate_procedures():
        magic_segment = ida_segment_name(segment_token(magic_id))
        wizards_segment = ida_segment_name(segment_token(wizards_id))
        segment_pair = (magic_segment, wizards_segment)
        if segment_pair not in duplicate_segments:
            continue
        if segment_pair in ignore or segment_pair in special:
            continue
        if ("WIZARDS", wizards_segment) not in owned:
            continue
        old_name = procedure_name(magic_id)
        new_name = procedure_name(wizards_id)
        if old_name == new_name or old_name == "missing" or new_name == "missing":
            continue
        renames.append((magic_id, wizards_id, old_name, new_name))
    return renames


def render_fwv(renames: Sequence[Rename]) -> str:
    rows = [(magic_id, wizards_id, old_name, new_name) for magic_id, wizards_id, old_name, new_name in renames]
    widths = [
        max([len("MAGIC")] + [len(row[0]) for row in rows]),
        max([len("WIZARDS")] + [len(row[1]) for row in rows]),
        max([len("OLD_NAME")] + [len(row[2]) for row in rows]),
    ]
    lines = [f"{'MAGIC':<{widths[0]}}  {'WIZARDS':<{widths[1]}}  {'OLD_NAME':<{widths[2]}}  NEW_NAME"]
    for magic_id, wizards_id, old_name, new_name in rows:
        lines.append(f"{magic_id:<{widths[0]}}  {wizards_id:<{widths[1]}}  {old_name:<{widths[2]}}  {new_name}")
    return "\n".join(lines) + "\n"


def escape_idc_string(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def render_idc(renames: Sequence[Rename]) -> str:
    lines = [
        "#include <idc.idc>",
        "",
        "static rename_mgc_proc(old_name, new_name)",
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
    for _magic_id, _wizards_id, old_name, new_name in renames:
        lines.append(
            '    attempted = attempted + 1; ok = ok + rename_mgc_proc("%s", "%s");'
            % (escape_idc_string(old_name), escape_idc_string(new_name))
        )
    lines.extend(
        [
            "",
            "    Message(\"mgc-rename-from-wzd-summary,attempted=%d,ok=%d,failed=%d\\n\", attempted, ok, attempted - ok);",
            "    Batch(0);",
            "    if ( ok == attempted )",
            "        Exit(0);",
            "    Exit(1);",
            "}",
            "",
        ]
    )
    return "\n".join(lines)


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def main() -> int:
    renames = rename_plan()
    write_text(RENAME_PLAN, render_fwv(renames))
    write_text(RENAME_IDC, render_idc(renames))
    print(f"Wrote {RENAME_PLAN} ({len(renames)} rows)")
    print(f"Wrote {RENAME_IDC} ({len(renames)} renames)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
