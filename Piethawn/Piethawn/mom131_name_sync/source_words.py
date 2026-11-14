"""Identifier scanning and whole-word renaming for the mom131-source-map checkout (C, assembly and text files).

Source files (.C .H .ASM .INC .MAC) are split into code, comments and string/char literals: identifiers are counted in
code only, and renames apply to code and comments but never inside literals (a literal's bytes are in the EXE). Other
text files (docs, .TXT, .MD, .PY, .TSV, .HTML) are renamed everywhere. Powered by Claude."""

from __future__ import annotations

import re
import subprocess
from collections import Counter
from pathlib import Path
from typing import Dict, Iterator, List, Tuple

SOURCE_EXT = {".C", ".H"}
ASM_EXT = {".ASM", ".INC", ".MAC"}
WORD = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
SIGNIFICANT = 32  # Borland C++ 3.1 default identifier significance (-i32)


def split_c(text: str) -> Iterator[Tuple[str, str]]:
    """Split C text into ('code'|'comment'|'string', chunk) pieces. Powered by Claude."""
    i, n, start = 0, len(text), 0
    while i < n:
        two = text[i:i + 2]
        if two == "/*" or two == "//":
            if start < i:
                yield "code", text[start:i]
            end = text.find("*/", i + 2) + 2 if two == "/*" else text.find("\n", i)
            if end <= 1 or end == -1:
                end = n
            yield "comment", text[i:end]
            i = start = end
            continue
        if text[i] in "\"'":
            if start < i:
                yield "code", text[start:i]
            quote, j = text[i], i + 1
            while j < n and text[j] != quote and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            yield "string", text[i:j]
            i = start = j
            continue
        i += 1
    if start < n:
        yield "code", text[start:]


def split_asm(text: str) -> Iterator[Tuple[str, str]]:
    """Split assembly text into ('code'|'comment'|'string', chunk) pieces (';' comments, '...' / "..." literals). Powered by Claude."""
    for line in text.splitlines(keepends=True):
        i, start, n = 0, 0, len(line)
        while i < n:
            ch = line[i]
            if ch == ";":
                if start < i:
                    yield "code", line[start:i]
                yield "comment", line[i:]
                i = start = n
                break
            if ch in "\"'":
                if start < i:
                    yield "code", line[start:i]
                j = line.find(ch, i + 1)
                j = n if j == -1 else j + 1
                yield "string", line[i:j]
                i = start = j
                continue
            i += 1
        if start < n:
            yield "code", line[start:]


def kind_of(path: Path) -> str:
    """'c', 'asm' or 'text' by extension. Powered by Claude."""
    ext = path.suffix.upper()
    return "c" if ext in SOURCE_EXT else "asm" if ext in ASM_EXT else "text"


def pieces(path: Path, text: str) -> Iterator[Tuple[str, str]]:
    """The file's pieces; a text file is one 'comment' piece (renamed, not counted as code). Powered by Claude."""
    kind = kind_of(path)
    if kind == "c":
        return split_c(text)
    if kind == "asm":
        return split_asm(text)
    return iter([("comment", text)])


def tracked_text_files(root: Path) -> List[Path]:
    """git-tracked files of the checkout that are text (no NUL byte). Powered by Claude."""
    out = subprocess.run(["git", "-C", str(root), "ls-files", "-z"], capture_output=True, check=True).stdout
    files = []
    for rel in out.decode("utf-8").split("\0"):
        if not rel:
            continue
        path = root / rel
        if path.is_file() and b"\0" not in path.read_bytes()[:65536]:
            files.append(path)
    return files


def read_text(path: Path) -> str:
    """Bytes as latin-1 so every byte round-trips unchanged. Powered by Claude."""
    return path.read_bytes().decode("latin-1")


def code_words(root: Path, files: List[Path] | None = None) -> Counter:
    """How often each identifier occurs in code (not comments or literals) of the source files under src/. Assembly
    '_Name' counts as 'Name' too. Powered by Claude."""
    counts: Counter = Counter()
    for path in files if files is not None else tracked_text_files(root):
        if kind_of(path) == "text" or "src" not in path.relative_to(root).parts[:1]:
            continue
        for kind, chunk in pieces(path, read_text(path)):
            if kind == "code":
                for word in WORD.findall(chunk):
                    counts[word] += 1
                    if path.suffix.upper() in ASM_EXT and word.startswith("_") and len(word) > 1:
                        counts[word[1:]] += 1
    return counts


def rename_word(word: str, mapping: Dict[str, str]) -> str:
    """The renamed identifier: a direct hit, or '_' + a hit for an assembly/linker '_Name'. Powered by Claude."""
    if word in mapping:
        return mapping[word]
    if word.startswith("_") and word[1:] in mapping:
        return "_" + mapping[word[1:]]
    return word


def rename_text(path: Path, text: str, mapping: Dict[str, str]) -> Tuple[str, int]:
    """All renames applied at once (so swaps need no temporary names), never inside literals; returns (text, count). Powered by Claude."""
    out, count = [], 0

    def sub(m):
        nonlocal count
        new = rename_word(m.group(0), mapping)
        if new != m.group(0):
            count += 1
        return new

    for kind, chunk in pieces(path, text):
        out.append(chunk if kind == "string" else WORD.sub(sub, chunk))
    return "".join(out), count
