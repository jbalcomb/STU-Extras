#!/usr/bin/env python3
"""
Dump likely IDA Pro 5.5 registry settings to JSON.

This script is intentionally registry-only. It looks for:
- vendor keys under Hex-Rays / Ilfak
- uninstall entries that point to an IDA install
- IDA-related values under App Paths

It also highlights values that are likely to affect generated ASM output.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from dataclasses import asdict, dataclass, field
from typing import Dict, Iterator, List, Optional, Sequence, Tuple

try:
    import winreg
except ImportError:  # pragma: no cover
    winreg = None  # type: ignore[assignment]


REGISTRY_ROOTS: Sequence[Tuple[str, object]] = (
    ("HKCU", getattr(winreg, "HKEY_CURRENT_USER", None)),
    ("HKLM", getattr(winreg, "HKEY_LOCAL_MACHINE", None)),
)

VENDOR_ROOTS: Sequence[str] = (
    r"Software\Hex-Rays",
    r"Software\Ilfak",
    r"Software\Wow6432Node\Hex-Rays",
    r"Software\Wow6432Node\Ilfak",
)

UNINSTALL_ROOTS: Sequence[str] = (
    r"Software\Microsoft\Windows\CurrentVersion\Uninstall",
    r"Software\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall",
)

APP_PATHS_ROOTS: Sequence[str] = (
    r"Software\Microsoft\Windows\CurrentVersion\App Paths",
    r"Software\Wow6432Node\Microsoft\Windows\CurrentVersion\App Paths",
)

IDA_NAME_RE = re.compile(r"\bida\b|\bida pro\b|\bhex-rays\b|\bilfak\b", re.IGNORECASE)
SETTING_HINT_RE = re.compile(
    r"asm|list|output|comment|xref|indent|tab|demang|segment|proc|name|dir|path|cfg|config",
    re.IGNORECASE,
)


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


def enum_values(key) -> List[RegistryValue]:
    results: List[RegistryValue] = []
    index = 0
    while True:
        try:
            name, value, value_type = winreg.EnumValue(key, index)
        except OSError:
            break
        results.append(RegistryValue(name=name, value=value, value_type=value_type))
        index += 1
    return results


def enum_subkeys(key) -> List[str]:
    results: List[str] = []
    index = 0
    while True:
        try:
            results.append(winreg.EnumKey(key, index))
        except OSError:
            break
        index += 1
    return results


def open_key_dump(hive_name: str, hive, path: str) -> Optional[RegistryKeyDump]:
    if winreg is None or hive is None:
        return None

    try:
        with winreg.OpenKey(hive, path) as key:
            return RegistryKeyDump(
                hive=hive_name,
                path=path,
                values=enum_values(key),
                subkeys=enum_subkeys(key),
            )
    except OSError:
        return None


def walk_tree(hive_name: str, hive, root_path: str) -> Iterator[RegistryKeyDump]:
    root = open_key_dump(hive_name, hive, root_path)
    if root is None:
        return

    yield root
    for subkey in root.subkeys:
        yield from walk_tree(hive_name, hive, root_path + "\\" + subkey)


def vendor_keys() -> List[RegistryKeyDump]:
    results: List[RegistryKeyDump] = []
    seen: set[Tuple[str, str]] = set()

    for hive_name, hive in REGISTRY_ROOTS:
        for root_path in VENDOR_ROOTS:
            for dump in walk_tree(hive_name, hive, root_path):
                key_id = (dump.hive, dump.path)
                if key_id in seen:
                    continue
                seen.add(key_id)
                results.append(dump)
    return results


def uninstall_keys() -> List[RegistryKeyDump]:
    results: List[RegistryKeyDump] = []

    for hive_name, hive in REGISTRY_ROOTS:
        for uninstall_root in UNINSTALL_ROOTS:
            parent = open_key_dump(hive_name, hive, uninstall_root)
            if parent is None:
                continue
            for subkey in parent.subkeys:
                dump = open_key_dump(hive_name, hive, uninstall_root + "\\" + subkey)
                if dump is None:
                    continue
                values = {item.name: item.value for item in dump.values}
                haystack = " ".join(
                    str(values.get(name, ""))
                    for name in ("DisplayName", "Publisher", "InstallLocation", "DisplayIcon")
                )
                if IDA_NAME_RE.search(haystack):
                    results.append(dump)
    return results


def app_path_keys() -> List[RegistryKeyDump]:
    results: List[RegistryKeyDump] = []

    for hive_name, hive in REGISTRY_ROOTS:
        for app_paths_root in APP_PATHS_ROOTS:
            parent = open_key_dump(hive_name, hive, app_paths_root)
            if parent is None:
                continue
            for subkey in parent.subkeys:
                if not IDA_NAME_RE.search(subkey):
                    continue
                dump = open_key_dump(hive_name, hive, app_paths_root + "\\" + subkey)
                if dump is not None:
                    results.append(dump)
    return results


def hinted_values(dumps: Sequence[RegistryKeyDump]) -> List[Dict[str, object]]:
    results: List[Dict[str, object]] = []
    for dump in dumps:
        for value in dump.values:
            joined = f"{value.name} {value.value}"
            if SETTING_HINT_RE.search(joined):
                results.append(
                    {
                        "hive": dump.hive,
                        "path": dump.path,
                        "name": value.name,
                        "value": value.value,
                        "value_type": value.value_type,
                    }
                )
    return results


def install_locations(dumps: Sequence[RegistryKeyDump]) -> List[Dict[str, object]]:
    results: List[Dict[str, object]] = []
    seen: set[str] = set()

    for dump in dumps:
        values = {item.name: item.value for item in dump.values}
        for field_name in ("InstallLocation", "InstPath", "Path", "DisplayIcon", ""):
            raw = values.get(field_name)
            if raw is None:
                continue
            text = os.path.expandvars(str(raw)).strip().strip('"')
            if not text or not IDA_NAME_RE.search(text):
                continue
            if text in seen:
                continue
            seen.add(text)
            results.append(
                {
                    "source": f"{dump.hive}\\{dump.path}",
                    "field": field_name or "(default)",
                    "value": text,
                }
            )
    return results


def serialize(dumps: Sequence[RegistryKeyDump]) -> List[Dict[str, object]]:
    return [
        {
            "hive": dump.hive,
            "path": dump.path,
            "values": [asdict(value) for value in dump.values],
            "subkeys": dump.subkeys,
        }
        for dump in dumps
    ]


def build_report() -> Dict[str, object]:
    if winreg is None:
        raise RuntimeError("This script requires Windows and the winreg module.")

    vendor = vendor_keys()
    uninstall = uninstall_keys()
    app_paths = app_path_keys()
    all_dumps = [*vendor, *uninstall, *app_paths]

    return {
        "platform": sys.platform,
        "vendor_keys": serialize(vendor),
        "uninstall_keys": serialize(uninstall),
        "app_paths_keys": serialize(app_paths),
        "likely_setting_values": hinted_values(all_dumps),
        "likely_install_locations": install_locations(all_dumps),
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Dump likely IDA Pro 5.5 registry settings to JSON.")
    parser.add_argument(
        "-o",
        "--output",
        default="ida55_registry_settings.json",
        help="Output JSON path. Default: ida55_registry_settings.json",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    report = build_report()
    with open(args.output, "w", encoding="utf-8") as handle:
        json.dump(report, handle, indent=2, sort_keys=True)
    print(f"Wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
