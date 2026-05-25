#!/usr/bin/env python3
"""Validate IDA 5.5 ASM exports against the clean-export profile."""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, List, Optional, Sequence


PROC_START_RE = re.compile(r"^\s*(?:proc\s+\S+|\S+\s+.*\bproc\b)", re.IGNORECASE)
PROC_END_RE = re.compile(r"^\s*(?:endp\s+\S+|\S+\s+.*\bendp\b)", re.IGNORECASE)
SEGMENT_RE = re.compile(r"^\s*(?:segment\s+\S+|\S+\s+segment\b)", re.IGNORECASE)
IDA_LINE_PREFIX_RE = re.compile(r"^\s*(?:[A-Za-z_@$?][\w@$?]*|[0-9A-F]{4,}):[0-9A-F]{4,}\b")
OPCODE_PREFIX_RE = re.compile(
    r"^\s*(?:[A-Za-z_@$?][\w@$?]*:)?(?:[0-9A-F]{4,}:[0-9A-F]{4,}\s+)?"
    r"(?:[0-9A-F]{2}\s+){2,}[A-Za-z_.$@?][\w.$@?]*\b",
)
XREF_COMMENT_RE = re.compile(r";.*\b(?:CODE|DATA)?\s*XREF\b", re.IGNORECASE)
BORDER_RE = re.compile(r"^\s*;\s*-{10,}\s*$")


@dataclass
class Finding:
    path: Path
    line_number: int
    rule: str
    message: str
    line: str

    def format(self) -> str:
        location = f"{self.path}:{self.line_number}" if self.line_number else str(self.path)
        text = self.line.rstrip("\r\n")
        if text:
            return f"{location}: {self.rule}: {self.message}: {text}"
        return f"{location}: {self.rule}: {self.message}"


@dataclass
class AsmCheckResult:
    path: Path
    line_count: int = 0
    proc_count: int = 0
    endp_count: int = 0
    segment_count: int = 0
    findings: List[Finding] = field(default_factory=list)

    @property
    def ok(self) -> bool:
        return not self.findings


def add_file_finding(result: AsmCheckResult, rule: str, message: str) -> None:
    result.findings.append(Finding(result.path, 0, rule, message, ""))


def check_asm_text(path: Path, text: str, min_procedures: int = 1) -> AsmCheckResult:
    result = AsmCheckResult(path=path)
    for line_number, line in enumerate(text.splitlines(keepends=True), start=1):
        result.line_count += 1
        if "\t" in line:
            result.findings.append(Finding(path, line_number, "INF_ENTAB", "tab character found", line))
        if IDA_LINE_PREFIX_RE.search(line):
            result.findings.append(Finding(path, line_number, "INF_SHOWPREF", "IDA address line prefix found", line))
        if OPCODE_PREFIX_RE.search(line):
            result.findings.append(Finding(path, line_number, "INF_BINPREF", "opcode-byte prefix column found", line))
        if XREF_COMMENT_RE.search(line):
            result.findings.append(Finding(path, line_number, "INF_XREFNUM", "XREF comment found", line))
        if BORDER_RE.search(line):
            result.findings.append(Finding(path, line_number, "INF_BORDER", "data/code border line found", line))
        if PROC_START_RE.search(line):
            result.proc_count += 1
        if PROC_END_RE.search(line):
            result.endp_count += 1
        if SEGMENT_RE.search(line):
            result.segment_count += 1

    if result.line_count == 0:
        add_file_finding(result, "file", "ASM file is empty")
    if result.proc_count < min_procedures:
        add_file_finding(result, "proc", f"expected at least {min_procedures} procedure(s), found {result.proc_count}")
    if result.proc_count != result.endp_count:
        add_file_finding(result, "proc", f"proc/endp count mismatch: {result.proc_count} proc, {result.endp_count} endp")

    return result


def check_asm_file(path: Path, min_procedures: int = 1) -> AsmCheckResult:
    if not path.exists():
        result = AsmCheckResult(path=path)
        add_file_finding(result, "file", "ASM file does not exist")
        return result
    if not path.is_file():
        result = AsmCheckResult(path=path)
        add_file_finding(result, "file", "ASM path is not a file")
        return result
    text = path.read_text(encoding="utf-8", errors="replace")
    return check_asm_text(path, text, min_procedures=min_procedures)


def print_result(result: AsmCheckResult) -> None:
    print(
        f"{result.path}: lines={result.line_count} "
        f"segments={result.segment_count} proc={result.proc_count} endp={result.endp_count}"
    )
    for finding in result.findings:
        print(f"- {finding.format()}")


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Check IDA 5.5 ASM exports for clean-profile formatting.")
    parser.add_argument("asm_files", nargs="+", type=Path, help="ASM file(s) to validate.")
    parser.add_argument("--min-procedures", type=int, default=1, help="Minimum procedure count per ASM file. Default: 1")
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    results = [check_asm_file(path, min_procedures=args.min_procedures) for path in args.asm_files]
    for result in results:
        print_result(result)

    failures = sum(1 for result in results if not result.ok)
    if failures:
        print(f"Clean ASM check failed: {failures} file(s) had findings.", file=sys.stderr)
        return 1

    print(f"Clean ASM check passed: {len(results)} file(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
