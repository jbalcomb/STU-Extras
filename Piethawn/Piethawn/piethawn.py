#!/usr/bin/env python3
"""
Collect likely IDA Pro 5.5 settings from the Windows registry and install files.

This does not attempt to automate IDA itself. Some values that affect ASM output
live in the current database or session state; use the companion IDC script in
this repo to dump those from inside IDA.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Dict, Iterable, Iterator, List, Optional, Sequence, Tuple

try:
    import winreg
except ImportError:  # pragma: no cover - non-Windows fallback
    winreg = None  # type: ignore[assignment]


REGISTRY_ROOTS: Sequence[Tuple[str, object]] = (
    ("HKCU", getattr(winreg, "HKEY_CURRENT_USER", None)),
    ("HKLM", getattr(winreg, "HKEY_LOCAL_MACHINE", None)),
)

IDA_VENDOR_PATHS: Sequence[str] = (
    r"Software\Hex-Rays",
    r"Software\Ilfak",
    r"Software\Wow6432Node\Hex-Rays",
    r"Software\Wow6432Node\Ilfak",
)

UNINSTALL_PATHS: Sequence[str] = (
    r"Software\Microsoft\Windows\CurrentVersion\Uninstall",
    r"Software\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall",
)

IDA_NAME_PATTERN = re.compile(r"\bIDA\b|\bIDA Pro\b|\bHex-Rays\b", re.IGNORECASE)
CONFIG_NAME_PATTERN = re.compile(r"^ida.*\.(cfg|cld|til|sig|xml|ini)$", re.IGNORECASE)


@dataclass
class RegistryValue:
    name: str
    value: object
    value_type: int


@dataclass
class RegistryKeyDump:
    hive: str
    path: str
    values: List[RegistryValue] = field(default_factory=list)
    subkeys: List[str] = field(default_factory=list)


@dataclass
class ConfigSnippet:
    path: str
    interesting_lines: List[str]


@dataclass
class InstallCandidate:
    source: str
    install_dir: str
    display_name: Optional[str] = None
    display_version: Optional[str] = None


def enum_registry_values(key) -> List[RegistryValue]:
    values: List[RegistryValue] = []
    index = 0
    while True:
        try:
            name, value, value_type = winreg.EnumValue(key, index)
        except OSError:
            break
        values.append(RegistryValue(name=name, value=value, value_type=value_type))
        index += 1
    return values


def enum_registry_subkeys(key) -> List[str]:
    subkeys: List[str] = []
    index = 0
    while True:
        try:
            subkeys.append(winreg.EnumKey(key, index))
        except OSError:
            break
        index += 1
    return subkeys


def read_registry_key(hive_name: str, hive, path: str) -> Optional[RegistryKeyDump]:
    if hive is None or winreg is None:
        return None

    try:
        with winreg.OpenKey(hive, path) as key:
            return RegistryKeyDump(
                hive=hive_name,
                path=path,
                values=enum_registry_values(key),
                subkeys=enum_registry_subkeys(key),
            )
    except OSError:
        return None


def iter_registry_tree(hive_name: str, hive, start_path: str) -> Iterator[RegistryKeyDump]:
    root_dump = read_registry_key(hive_name, hive, start_path)
    if root_dump is None:
        return

    yield root_dump
    for subkey_name in root_dump.subkeys:
        child_path = start_path + "\\" + subkey_name
        yield from iter_registry_tree(hive_name, hive, child_path)


def find_vendor_keys() -> List[RegistryKeyDump]:
    matches: List[RegistryKeyDump] = []
    seen: set[Tuple[str, str]] = set()

    for hive_name, hive in REGISTRY_ROOTS:
        for vendor_path in IDA_VENDOR_PATHS:
            for dump in iter_registry_tree(hive_name, hive, vendor_path):
                key_id = (dump.hive, dump.path)
                if key_id in seen:
                    continue
                seen.add(key_id)
                matches.append(dump)

    return matches


def find_uninstall_entries() -> List[RegistryKeyDump]:
    matches: List[RegistryKeyDump] = []

    for hive_name, hive in REGISTRY_ROOTS:
        for uninstall_path in UNINSTALL_PATHS:
            parent = read_registry_key(hive_name, hive, uninstall_path)
            if parent is None:
                continue

            for subkey_name in parent.subkeys:
                subkey_path = uninstall_path + "\\" + subkey_name
                dump = read_registry_key(hive_name, hive, subkey_path)
                if dump is None:
                    continue

                values = {item.name: item.value for item in dump.values}
                searchable = " ".join(
                    str(values.get(name, "")) for name in ("DisplayName", "Publisher", "InstallLocation")
                )
                if IDA_NAME_PATTERN.search(searchable):
                    matches.append(dump)

    return matches


def extract_install_candidates(registry_dumps: Sequence[RegistryKeyDump]) -> List[InstallCandidate]:
    results: List[InstallCandidate] = []
    seen: set[str] = set()

    for dump in registry_dumps:
        values = {item.name: item.value for item in dump.values}
        raw_dir = values.get("InstallLocation") or values.get("InstPath") or values.get("Path")
        if not raw_dir:
            continue

        install_dir = os.path.expandvars(str(raw_dir)).strip().strip('"')
        if not install_dir or install_dir in seen:
            continue

        seen.add(install_dir)
        results.append(
            InstallCandidate(
                source=f"{dump.hive}\\{dump.path}",
                install_dir=install_dir,
                display_name=str(values.get("DisplayName", "")) or None,
                display_version=str(values.get("DisplayVersion", "")) or None,
            )
        )

    return results


def candidate_config_paths(install_dir: Path) -> Iterator[Path]:
    common_names = (
        "ida.cfg",
        "idagui.cfg",
        "ida64.cfg",
        "plugins.cfg",
        "cfg/ida.cfg",
        "cfg/idagui.cfg",
        "cfg/ida64.cfg",
        "cfg/plugins.cfg",
    )
    for relative_name in common_names:
        candidate = install_dir / relative_name
        if candidate.exists():
            yield candidate

    for child in install_dir.rglob("*"):
        if child.is_file() and CONFIG_NAME_PATTERN.match(child.name):
            yield child


def extract_interesting_lines(path: Path) -> List[str]:
    interesting: List[str] = []
    seen: set[str] = set()
    keywords = (
        "asm",
        "xref",
        "comment",
        "segment",
        "proc",
        "endp",
        "name",
        "demangle",
        "indent",
        "directive",
        "output",
        "listing",
    )

    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return interesting

    for line in text.splitlines():
        stripped = line.strip()
        lower = stripped.lower()
        if not stripped or stripped.startswith("//") or stripped.startswith(";"):
            continue
        if any(keyword in lower for keyword in keywords):
            if stripped not in seen:
                seen.add(stripped)
                interesting.append(stripped)
    return interesting[:400]


def collect_config_snippets(install_dirs: Sequence[InstallCandidate]) -> List[ConfigSnippet]:
    snippets: List[ConfigSnippet] = []
    seen: set[str] = set()

    for candidate in install_dirs:
        base = Path(candidate.install_dir)
        if not base.exists():
            continue

        for config_path in candidate_config_paths(base):
            resolved = str(config_path.resolve())
            if resolved in seen:
                continue
            seen.add(resolved)
            snippets.append(
                ConfigSnippet(
                    path=resolved,
                    interesting_lines=extract_interesting_lines(config_path),
                )
            )

    return snippets


def collect_environment_hints() -> Dict[str, object]:
    hints: Dict[str, object] = {
        "platform": sys.platform,
        "is_windows": sys.platform.startswith("win"),
        "program_files": os.environ.get("ProgramFiles"),
        "program_files_x86": os.environ.get("ProgramFiles(x86)"),
    }

    likely_dirs = []
    for env_name in ("ProgramFiles", "ProgramFiles(x86)"):
        base = os.environ.get(env_name)
        if not base:
            continue
        base_path = Path(base)
        for child in base_path.glob("IDA*"):
            likely_dirs.append(str(child))
    hints["likely_install_dirs"] = likely_dirs
    return hints


def serialize_registry_dumps(dumps: Sequence[RegistryKeyDump]) -> List[Dict[str, object]]:
    serialized: List[Dict[str, object]] = []
    for dump in dumps:
        serialized.append(
            {
                "hive": dump.hive,
                "path": dump.path,
                "values": [asdict(value) for value in dump.values],
                "subkeys": dump.subkeys,
            }
        )
    return serialized


def build_report() -> Dict[str, object]:
    if winreg is None:
        raise RuntimeError("This script must be run on Windows to inspect the registry.")

    vendor_keys = find_vendor_keys()
    uninstall_keys = find_uninstall_entries()
    install_candidates = extract_install_candidates([*vendor_keys, *uninstall_keys])
    config_snippets = collect_config_snippets(install_candidates)

    return {
        "environment": collect_environment_hints(),
        "vendor_registry_keys": serialize_registry_dumps(vendor_keys),
        "uninstall_registry_keys": serialize_registry_dumps(uninstall_keys),
        "install_candidates": [asdict(item) for item in install_candidates],
        "config_snippets": [asdict(item) for item in config_snippets],
        "next_step": (
            "Run ida55_dump_settings.idc inside IDA Pro 5.5 and combine its output "
            "with this JSON when building the ASM parser."
        ),
    }


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Collect likely IDA Pro 5.5 settings from the registry and install files."
    )
    parser.add_argument(
        "-o",
        "--output",
        default="ida55_settings.json",
        help="Path to the JSON report to write. Defaults to ida55_settings.json in the current directory.",
    )
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    report = build_report()

    output_path = Path(args.output)
    output_path.write_text(json.dumps(report, indent=2, sort_keys=True), encoding="utf-8")
    print(f"Wrote {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
