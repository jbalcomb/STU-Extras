"""Which program (WZD = WIZARDS.EXE, MGC = MAGIC.EXE) each source file of the mom131-source-map checkout is part of.

  src/wizards/MODULES.TSV, src/magic/MODULES.TSV   the modules each program links (MAGIC's list names its shared
                                                   modules as ../wizards/X.C)
  src/ail/*                                        the AIL library, linked into both
  src/include/WIZDECL.H, MAGDECL.H                 the generated per-program declaration headers (no source includes
                                                   them; tools/declbuild.py compiles each program's sources with one)
  other headers                                    the programs of the files that #include them (transitively)

A file in both programs is "shared": a name used there must mean the same in both. Powered by Claude."""

from __future__ import annotations

import os
import re
from pathlib import Path
from typing import Dict, Set

PROGRAMS = ("WZD", "MGC")
MODULE_LISTS = {"WZD": "src/wizards/MODULES.TSV", "MGC": "src/magic/MODULES.TSV"}
DECL_HEADERS = {"src/include/WIZDECL.H": "WZD", "src/include/MAGDECL.H": "MGC"}
C_INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.MULTILINE)
ASM_INCLUDE = re.compile(r"^\s*include\s+(\S+)", re.MULTILINE | re.IGNORECASE)


def rel(root: Path, path: Path) -> str:
    """Checkout-relative path with forward slashes. Powered by Claude."""
    return os.path.relpath(path, root).replace(os.sep, "/")


def module_files(root: Path, program: str) -> Set[str]:
    """The source files a program links, from its MODULES.TSV, plus src/ail. Powered by Claude."""
    listing = root / MODULE_LISTS[program]
    files = set()
    for line in listing.read_text(encoding="latin-1").splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        name = line.split("\t")[-1].strip()
        path = (listing.parent / name).resolve()
        if not path.is_file():
            raise FileNotFoundError(f"{listing}: module {name} not found at {path}")
        files.add(rel(root, path))
    for path in (root / "src" / "ail").glob("*"):
        if path.suffix.upper() in (".ASM", ".INC", ".MAC"):
            files.add(rel(root, path))
    return files


def find_include(root: Path, including: Path, name: str) -> str:
    """Resolve an #include / INCLUDE: next to the file, then src/include (case-insensitive). Powered by Claude."""
    for folder in (including.parent, root / "src" / "include"):
        for candidate in folder.iterdir():
            if candidate.name.upper() == Path(name).name.upper():
                return rel(root, candidate)
    return ""


def program_files(root: Path) -> Dict[str, Set[str]]:
    """{program: checkout-relative source files} including headers. Powered by Claude."""
    result = {}
    for program in PROGRAMS:
        files = module_files(root, program)
        todo = list(files)
        while todo:
            path = root / todo.pop()
            text = path.read_text(encoding="latin-1")
            pattern = ASM_INCLUDE if path.suffix.upper() in (".ASM", ".INC", ".MAC") else C_INCLUDE
            for name in pattern.findall(text):
                found = find_include(root, path, name)
                if found and found not in files:
                    files.add(found)
                    todo.append(found)
        files |= {h for h, p in DECL_HEADERS.items() if p == program and (root / h).is_file()}
        result[program] = files
    return result


def file_programs(files: Dict[str, Set[str]]) -> Dict[str, Set[str]]:
    """{file: {programs}} from program_files(). Powered by Claude."""
    out: Dict[str, Set[str]] = {}
    for program, names in files.items():
        for name in names:
            out.setdefault(name, set()).add(program)
    return out
