#!/usr/bin/env python3
"""
Shared helpers for comparing split IDA ASM procedure files.
"""

from __future__ import annotations

import hashlib
import json
import re
from dataclasses import asdict, dataclass
from difflib import SequenceMatcher, unified_diff
from pathlib import Path
from typing import Dict, Iterable, Iterator, List, Optional, Sequence, Tuple


PROC_HEADER_RE = re.compile(r"^\s*proc\s+(?P<name>[A-Za-z_@$?][\w@$?]*)\b|^\s*proc\b\s*(?P<name2>[A-Za-z_@$?][\w@$?]*)\b", re.IGNORECASE)
ALT_PROC_HEADER_RE = re.compile(r"^\s*(?P<name>[A-Za-z_@$?][\w@$?]*)\s+proc\b", re.IGNORECASE)
COMMENT_ONLY_RE = re.compile(r"^\s*;")
BLANK_RE = re.compile(r"^\s*$")
INLINE_COMMENT_RE = re.compile(r"\s*;.*$")
WHITESPACE_RE = re.compile(r"\s+")
AUTO_LABEL_RE = re.compile(r"\b(?:loc|sub|off|byte|word|dword|stru|unk|asc|algn)_[0-9A-Fa-f]+\b")
LOCAL_LABEL_RE = re.compile(r"@@[A-Za-z0-9_.$?]+")
HEX_NUMBER_RE = re.compile(r"\b[0-9A-FA-F]+h\b", re.IGNORECASE)
DEC_NUMBER_RE = re.compile(r"\b\d+\b")
ANGLE_ADDR_RE = re.compile(r"<[^>]+>")
OPERAND_SPLIT_RE = re.compile(r"\s*,\s*")


CODE_SEGMENTS = {"SOUND", "_AIL_", "_OVRTEXT_", "seg000"}
BORLAND_OVR_SEGMENTS = {"_OVRTEXT_", "_OVRDATA_", "_STUB_", "_EXEINFO_", "_EXTSEG_", "_EMSSEG_", "_VDISKSEG_"}
AIL_SEGMENTS = {"SOUND", "_AIL_"}


@dataclass
class ProcRecord:
    game: str
    segment: str
    relative_path: str
    absolute_path: str
    proc_name: str
    library_family: str
    raw_sha256: str
    normalized_sha256: str
    mnemonic_sha256: str
    raw_text: str
    normalized_text: str
    mnemonic_text: str
    raw_line_count: int
    normalized_line_count: int
    instruction_count: int
    comment_lines: List[str]


def proc_name_from_text(text: str, fallback: str) -> str:
    for line in text.splitlines():
        match = PROC_HEADER_RE.match(line)
        if match:
            return match.group("name") or match.group("name2") or fallback
        match = ALT_PROC_HEADER_RE.match(line)
        if match:
            return match.group("name")
    return fallback


def classify_library_family(game: str, segment: str, proc_name: str, text: str) -> str:
    if segment in AIL_SEGMENTS:
        return "AIL"
    if segment in BORLAND_OVR_SEGMENTS:
        return "BORLAND_OVR"
    lowered = text.lower()
    if "source: borlandc\\" in lowered or "library function" in lowered:
        return "BORLAND_RTL"
    if "source: miles" in lowered or "_ail_" in lowered or "ail.asm" in lowered:
        return "AIL"
    return "GAME"


def iter_proc_files(root: Path) -> Iterator[Tuple[str, str, Path]]:
    for game_dir in (root / "MAGIC", root / "WIZARDS"):
        if not game_dir.exists():
            continue
        game = game_dir.name
        for path in sorted(game_dir.rglob("*.asm")):
            if path.name == "_misc.asm":
                continue
            try:
                segment = path.relative_to(game_dir).parts[0]
            except ValueError:
                continue
            yield game, segment, path


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def extract_comment_lines(text: str) -> List[str]:
    return [line.strip() for line in text.splitlines() if COMMENT_ONLY_RE.match(line)]


def normalize_operand(operand: str) -> str:
    operand = operand.strip().lower()
    operand = LOCAL_LABEL_RE.sub("@@LOCAL", operand)
    operand = AUTO_LABEL_RE.sub("AUTO_LABEL", operand)
    operand = ANGLE_ADDR_RE.sub("<ANGLE>", operand)
    operand = HEX_NUMBER_RE.sub("HEX", operand)
    operand = DEC_NUMBER_RE.sub("DEC", operand)
    operand = WHITESPACE_RE.sub(" ", operand)
    return operand.strip()


def split_mnemonic_operands(line: str) -> Optional[Tuple[str, List[str]]]:
    stripped = line.strip()
    if not stripped:
        return None
    if stripped.endswith(":"):
        return None
    parts = stripped.split(None, 1)
    mnemonic = parts[0].lower()
    if mnemonic in {"proc", "endp", "db", "dw", "dd", "align", "assume", "public", "extrn", "segment", "ends"}:
        return None
    if len(parts) == 1:
        return mnemonic, []
    operands = [normalize_operand(item) for item in OPERAND_SPLIT_RE.split(parts[1])]
    return mnemonic, operands


def normalize_asm(text: str, *, strict: bool = False) -> Tuple[str, str]:
    normalized_lines: List[str] = []
    mnemonic_lines: List[str] = []
    for raw_line in text.splitlines():
        line = raw_line.rstrip()
        if BLANK_RE.match(line):
            continue
        if COMMENT_ONLY_RE.match(line):
            if strict:
                continue
            comment_text = line.lstrip(";").strip()
            if comment_text.startswith("-"):
                continue
            normalized_lines.append(f"; {comment_text.lower()}")
            continue

        line = INLINE_COMMENT_RE.sub("", line).strip()
        if not line:
            continue
        line = LOCAL_LABEL_RE.sub("@@LOCAL", line)
        line = AUTO_LABEL_RE.sub("AUTO_LABEL", line)
        line = HEX_NUMBER_RE.sub("HEX", line)
        if not strict:
            line = DEC_NUMBER_RE.sub("DEC", line)
        line = WHITESPACE_RE.sub(" ", line)
        normalized_lines.append(line.lower())

        parsed = split_mnemonic_operands(line)
        if parsed is None:
            continue
        mnemonic, operands = parsed
        mnemonic_lines.append(" ".join([mnemonic, *operands]).strip())

    return "\n".join(normalized_lines), "\n".join(mnemonic_lines)


def load_proc_record(root: Path, game: str, segment: str, path: Path) -> ProcRecord:
    text = path.read_text(encoding="utf-8", errors="replace")
    proc_name = proc_name_from_text(text, path.stem)
    normalized_text, mnemonic_text = normalize_asm(text)
    library_family = classify_library_family(game, segment, proc_name, text)
    return ProcRecord(
        game=game,
        segment=segment,
        relative_path=str(path.relative_to(root)).replace("\\", "/"),
        absolute_path=str(path),
        proc_name=proc_name,
        library_family=library_family,
        raw_sha256=sha256_text(text),
        normalized_sha256=sha256_text(normalized_text),
        mnemonic_sha256=sha256_text(mnemonic_text),
        raw_text=text,
        normalized_text=normalized_text,
        mnemonic_text=mnemonic_text,
        raw_line_count=len(text.splitlines()),
        normalized_line_count=len(normalized_text.splitlines()) if normalized_text else 0,
        instruction_count=len([line for line in mnemonic_text.splitlines() if line]),
        comment_lines=extract_comment_lines(text),
    )


def load_all_proc_records(root: Path) -> List[ProcRecord]:
    records: List[ProcRecord] = []
    for game, segment, path in iter_proc_files(root):
        records.append(load_proc_record(root, game, segment, path))
    return records


def similarity_score(left: str, right: str) -> float:
    return SequenceMatcher(None, left, right).ratio()


def comment_overlap(left: Sequence[str], right: Sequence[str]) -> List[str]:
    right_set = {item for item in right if item}
    return [item for item in left if item and item in right_set]


def candidate_similarity(left: ProcRecord, right: ProcRecord) -> float:
    mnemonic_score = similarity_score(left.mnemonic_text, right.mnemonic_text)
    normalized_score = similarity_score(left.normalized_text, right.normalized_text)
    return max(mnemonic_score, normalized_score)


def diff_category(left: ProcRecord, right: ProcRecord) -> str:
    if left.raw_sha256 == right.raw_sha256:
        return "identical_raw"
    if left.normalized_sha256 == right.normalized_sha256:
        return "identical_normalized"
    if left.mnemonic_sha256 == right.mnemonic_sha256:
        return "identical_mnemonic"
    score = candidate_similarity(left, right)
    if score >= 0.98:
        return "cosmetic_or_labels"
    if score >= 0.90:
        return "very_similar"
    if score >= 0.75:
        return "related"
    return "different"


def unified_normalized_diff(left: ProcRecord, right: ProcRecord, *, context: int = 3) -> str:
    return "".join(
        unified_diff(
            left.normalized_text.splitlines(keepends=True),
            right.normalized_text.splitlines(keepends=True),
            fromfile=left.relative_path,
            tofile=right.relative_path,
            n=context,
        )
    )


def canonical_key(record: ProcRecord) -> Tuple[int, str, str]:
    family_rank = {"AIL": 0, "BORLAND_OVR": 1, "BORLAND_RTL": 2, "GAME": 3}.get(record.library_family, 9)
    game_rank = {"WIZARDS": 0, "MAGIC": 1}.get(record.game, 9)
    return (family_rank, game_rank, record.relative_path)


def choose_canonical(records: Sequence[ProcRecord]) -> ProcRecord:
    return sorted(records, key=canonical_key)[0]


def json_default(value: object) -> object:
    if isinstance(value, ProcRecord):
        data = asdict(value)
        data.pop("raw_text", None)
        data.pop("normalized_text", None)
        data.pop("mnemonic_text", None)
        return data
    raise TypeError(f"Object of type {type(value)!r} is not JSON serializable")


def write_json(path: Path, payload: object) -> None:
    path.write_text(json.dumps(payload, indent=2, default=json_default), encoding="utf-8")
