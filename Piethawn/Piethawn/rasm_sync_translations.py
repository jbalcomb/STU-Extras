#!/usr/bin/env python3
"""Audit translated C files against current split ASM procedure names and HOWTO checklist."""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Sequence


DEFAULT_CHECKLIST = Path("HOWTO-DasmRasmPipeline.md")
CHECKLIST_RE = re.compile(
    r"^(?P<prefix>\s*\[)(?P<mark>[ xX]?)(?P<suffix>\]\s+.*?\brasm_named_gemini\.py\s+)(?P<name>[A-Za-z_][A-Za-z0-9_]*)(?P<rest>.*)$"
)


@dataclass(frozen=True)
class TranslationRecord:
    c_path: Path
    sibling_asm: Path
    same_name_asm: List[Path]

    @property
    def name(self) -> str:
        return self.c_path.stem

    @property
    def status(self) -> str:
        if self.sibling_asm.exists():
            return "matched"
        if len(self.same_name_asm) == 1:
            target_c = self.same_name_asm[0].with_suffix(".c")
            return "move-conflict" if target_c.exists() else "move-candidate"
        if len(self.same_name_asm) > 1:
            return "ambiguous"
        return "stale"

    @property
    def target_c(self) -> Optional[Path]:
        if self.status != "move-candidate":
            return None
        return self.same_name_asm[0].with_suffix(".c")


@dataclass(frozen=True)
class ChecklistEntry:
    line_number: int
    line: str
    function_name: str
    checked: bool


@dataclass(frozen=True)
class ChecklistUpdate:
    line_number: int
    function_name: str
    old_line: str
    new_line: str


def path_text(path: Path) -> str:
    return str(path).replace("\\", "/")


def normalize_path(path: Path, repo_root: Path) -> Path:
    path_text_value = str(path)
    if os.name != "nt":
        drive_match = re.match(r"^([A-Za-z]):[\\/](.*)$", path_text_value)
        if drive_match:
            drive = drive_match.group(1).lower()
            rest = drive_match.group(2).replace("\\", "/")
            return Path("/mnt") / drive / rest
    return path if path.is_absolute() else repo_root / path


def index_asm_by_stem(out_root: Path) -> Dict[str, List[Path]]:
    index: Dict[str, List[Path]] = {}
    for asm_path in sorted(out_root.glob("*/*/*.asm")):
        index.setdefault(asm_path.stem, []).append(asm_path)
    return index


def discover_translations(out_root: Path) -> List[TranslationRecord]:
    asm_index = index_asm_by_stem(out_root)
    records: List[TranslationRecord] = []
    for c_path in sorted(out_root.glob("*/*/*.c")):
        records.append(
            TranslationRecord(
                c_path=c_path,
                sibling_asm=c_path.with_suffix(".asm"),
                same_name_asm=asm_index.get(c_path.stem, []),
            )
        )
    return records


def translated_names(records: Sequence[TranslationRecord]) -> set[str]:
    return {record.name for record in records if record.status in {"matched", "move-candidate", "move-conflict", "ambiguous"}}


def parse_checklist(text: str) -> List[ChecklistEntry]:
    entries: List[ChecklistEntry] = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        match = CHECKLIST_RE.match(line)
        if not match:
            continue
        entries.append(
            ChecklistEntry(
                line_number=line_number,
                line=line,
                function_name=match.group("name"),
                checked=match.group("mark").lower() == "x",
            )
        )
    return entries


def plan_checklist_updates(entries: Sequence[ChecklistEntry], names_with_c: set[str]) -> List[ChecklistUpdate]:
    updates: List[ChecklistUpdate] = []
    for entry in entries:
        if entry.checked or entry.function_name not in names_with_c:
            continue
        match = CHECKLIST_RE.match(entry.line)
        if not match:
            continue
        updates.append(
            ChecklistUpdate(
                line_number=entry.line_number,
                function_name=entry.function_name,
                old_line=entry.line,
                new_line=f"{match.group('prefix')}x{match.group('suffix')}{match.group('name')}{match.group('rest')}",
            )
        )
    return updates


def apply_checklist_updates(checklist_path: Path, updates: Sequence[ChecklistUpdate]) -> None:
    if not updates:
        return
    lines = checklist_path.read_text(encoding="utf-8", errors="ignore").splitlines()
    by_line = {update.line_number: update for update in updates}
    new_lines = [by_line[index].new_line if index in by_line else line for index, line in enumerate(lines, start=1)]
    checklist_path.write_text("\n".join(new_lines) + "\n", encoding="utf-8")


def apply_renames(records: Sequence[TranslationRecord]) -> List[dict[str, str]]:
    applied: List[dict[str, str]] = []
    for record in records:
        target_c = record.target_c
        if target_c is None:
            continue
        target_c.parent.mkdir(parents=True, exist_ok=True)
        shutil.move(str(record.c_path), str(target_c))
        applied.append({"from": path_text(record.c_path), "to": path_text(target_c)})
    return applied


def render_record(record: TranslationRecord) -> dict[str, object]:
    return {
        "status": record.status,
        "c_path": path_text(record.c_path),
        "sibling_asm": path_text(record.sibling_asm),
        "same_name_asm": [path_text(path) for path in record.same_name_asm],
        "target_c": path_text(record.target_c) if record.target_c else None,
    }


def write_report(
    report_path: Path,
    records: Sequence[TranslationRecord],
    checklist_entries: Sequence[ChecklistEntry],
    checklist_updates: Sequence[ChecklistUpdate],
    applied_renames: Sequence[dict[str, str]],
) -> None:
    report_path.parent.mkdir(parents=True, exist_ok=True)
    status_counts: Dict[str, int] = {}
    for record in records:
        status_counts[record.status] = status_counts.get(record.status, 0) + 1
    names_with_c = translated_names(records)
    checked_without_c = [entry for entry in checklist_entries if entry.checked and entry.function_name not in names_with_c]

    document = {
        "summary": {
            "translation_count": len(records),
            "status_counts": status_counts,
            "checklist_entries": len(checklist_entries),
            "checklist_updates": len(checklist_updates),
            "checked_without_c": len(checked_without_c),
            "applied_renames": len(applied_renames),
        },
        "translations": [render_record(record) for record in records],
        "checklist_updates": [
            {
                "line_number": update.line_number,
                "function_name": update.function_name,
                "old_line": update.old_line,
                "new_line": update.new_line,
            }
            for update in checklist_updates
        ],
        "checked_without_c": [
            {
                "line_number": entry.line_number,
                "function_name": entry.function_name,
                "line": entry.line,
            }
            for entry in checked_without_c
        ],
        "applied_renames": list(applied_renames),
    }
    report_path.write_text(json.dumps(document, indent=2) + "\n", encoding="utf-8")


def print_summary(
    records: Sequence[TranslationRecord],
    checklist_entries: Sequence[ChecklistEntry],
    checklist_updates: Sequence[ChecklistUpdate],
    applied_renames: Sequence[dict[str, str]],
) -> None:
    status_counts: Dict[str, int] = {}
    for record in records:
        status_counts[record.status] = status_counts.get(record.status, 0) + 1

    print(f"Translated C files: {len(records)}")
    for status in sorted(status_counts):
        print(f"- {status}: {status_counts[status]}")

    interesting = [record for record in records if record.status != "matched"]
    if interesting:
        print("")
        print("Non-matched translations:")
        for record in interesting:
            print(f"- {record.status}: {record.c_path}")
            for asm_path in record.same_name_asm:
                print(f"  candidate ASM: {asm_path}")
            if record.target_c:
                print(f"  target C: {record.target_c}")

    if checklist_entries:
        names_with_c = translated_names(records)
        checked_without_c = [entry for entry in checklist_entries if entry.checked and entry.function_name not in names_with_c]
        print("")
        print(f"HOWTO checklist entries: {len(checklist_entries)}")
        print(f"- unchecked with translated C: {len(checklist_updates)}")
        print(f"- checked without translated C: {len(checked_without_c)}")
        if checklist_updates:
            print("")
            print("Checklist lines that can be marked translated:")
            for update in checklist_updates[:20]:
                print(f"- line {update.line_number}: {update.function_name}")
            if len(checklist_updates) > 20:
                print(f"- ... {len(checklist_updates) - 20} more")
        if checked_without_c:
            print("")
            print("Checked checklist entries without matching translated C:")
            for entry in checked_without_c[:20]:
                print(f"- line {entry.line_number}: {entry.function_name}")
            if len(checked_without_c) > 20:
                print(f"- ... {len(checked_without_c) - 20} more")

    if applied_renames:
        print("")
        print(f"Applied renames: {len(applied_renames)}")
        for item in applied_renames:
            print(f"- {item['from']} -> {item['to']}")


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Sync Piethawn translated C files with current split ASM names.")
    parser.add_argument("--out-root", type=Path, default=Path("out"), help="Split ASM/C output root. Default: out")
    parser.add_argument(
        "--checklist",
        type=Path,
        default=DEFAULT_CHECKLIST,
        help=f"Gemini translation checklist markdown. Default: {DEFAULT_CHECKLIST}",
    )
    parser.add_argument(
        "--report",
        type=Path,
        default=Path("out") / "rasm-translation-sync.json",
        help="JSON report path. Default: out/rasm-translation-sync.json",
    )
    parser.add_argument("--apply-renames", action="store_true", help="Move unambiguous stale C files next to their current ASM file.")
    parser.add_argument("--update-checklist", action="store_true", help="Mark HOWTO checklist entries checked when translated C exists.")
    parser.add_argument("--no-report", action="store_true", help="Do not write a JSON report.")
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    repo_root = Path(__file__).resolve().parent
    args.out_root = normalize_path(args.out_root, repo_root)
    args.checklist = normalize_path(args.checklist, repo_root)
    args.report = normalize_path(args.report, repo_root)

    records = discover_translations(args.out_root)
    checklist_entries = parse_checklist(args.checklist.read_text(encoding="utf-8", errors="ignore")) if args.checklist.exists() else []
    checklist_updates = plan_checklist_updates(checklist_entries, translated_names(records))

    applied_renames = apply_renames(records) if args.apply_renames else []
    if args.update_checklist:
        apply_checklist_updates(args.checklist, checklist_updates)

    if not args.no_report:
        write_report(args.report, records, checklist_entries, checklist_updates, applied_renames)

    print_summary(records, checklist_entries, checklist_updates, applied_renames)
    if not args.apply_renames:
        print("")
        print("Dry-run: pass --apply-renames to move unambiguous C files.")
    if not args.update_checklist:
        print("Dry-run: pass --update-checklist to mark HOWTO entries checked when translated C exists.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
