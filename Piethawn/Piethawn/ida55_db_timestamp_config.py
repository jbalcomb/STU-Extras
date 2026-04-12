#!/usr/bin/env python3
"""
Capture IDA Pro 5.5 database file timestamps into a JSON project config.
"""

from __future__ import annotations

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, List


DATABASE_BASENAMES = ("MAGIC", "WIZARDS")
DATABASE_EXTENSIONS = ("idb", "id0", "id1")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Save IDA Pro 5.5 database timestamps into a project config JSON.")
    parser.add_argument(
        "--ida-dir",
        default=r"C:\STU_DASM\IDA55_MOM131",
        help="Directory containing MAGIC/WIZARDS IDA 5.5 database files.",
    )
    parser.add_argument(
        "--output",
        default="project_config.json",
        help="Project config JSON to write or update. Default: project_config.json",
    )
    return parser.parse_args()


def iso_utc_from_timestamp(timestamp: float) -> str:
    return datetime.fromtimestamp(timestamp, tz=timezone.utc).isoformat().replace("+00:00", "Z")


def collect_file_info(path: Path) -> Dict[str, object]:
    stat = path.stat()
    return {
        "path": str(path),
        "file_name": path.name,
        "size_bytes": stat.st_size,
        "modified_timestamp": stat.st_mtime,
        "modified_utc": iso_utc_from_timestamp(stat.st_mtime),
    }


def collect_database_group(ida_dir: Path, base_name: str) -> Dict[str, object]:
    files: List[Dict[str, object]] = []
    missing: List[str] = []
    for extension in DATABASE_EXTENSIONS:
        path = ida_dir / f"{base_name}.{extension}"
        if path.exists():
            files.append(collect_file_info(path))
        else:
            missing.append(path.name)

    latest_modified = max((item["modified_timestamp"] for item in files), default=None)
    return {
        "name": base_name,
        "all_files_present": len(missing) == 0,
        "missing_files": missing,
        "files": files,
        "latest_modified_timestamp": latest_modified,
        "latest_modified_utc": iso_utc_from_timestamp(latest_modified) if latest_modified is not None else None,
    }


def load_existing_config(path: Path) -> Dict[str, object]:
    if not path.exists():
        return {}
    return json.loads(path.read_text(encoding="utf-8"))


def main() -> int:
    args = parse_args()
    ida_dir = Path(args.ida_dir)
    output_path = Path(args.output)

    groups = {base_name: collect_database_group(ida_dir, base_name) for base_name in DATABASE_BASENAMES}
    snapshot = {
        "captured_utc": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
        "ida_directory": str(ida_dir),
        "databases": groups,
    }

    config = load_existing_config(output_path)
    config["ida55_database_timestamps"] = snapshot

    output_path.write_text(json.dumps(config, indent=2), encoding="utf-8")
    print(f"Wrote {output_path}")
    for base_name, group in groups.items():
        print(
            f"{base_name}: latest={group['latest_modified_utc']} present={group['all_files_present']} missing={len(group['missing_files'])}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
