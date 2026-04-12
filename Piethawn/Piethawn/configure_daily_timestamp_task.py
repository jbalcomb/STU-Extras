#!/usr/bin/env python3
"""
Register a daily Windows Scheduled Task for the IDA database timestamp snapshot.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(
        description="Configure a daily Windows Scheduled Task to run ida55_db_timestamp_config.py."
    )
    parser.add_argument(
        "--task-name",
        default="Piethawn IDA55 Timestamp Snapshot",
        help="Scheduled task name.",
    )
    parser.add_argument(
        "--time",
        default="01:00",
        help="Daily run time in HH:MM 24-hour format. Default: 01:00",
    )
    parser.add_argument(
        "--python",
        default=sys.executable,
        help="Python interpreter to run inside the scheduled task. Default: current interpreter",
    )
    parser.add_argument(
        "--ida-dir",
        default=r"C:\STU_DASM\IDA55_MOM131",
        help="Directory containing the MAGIC/WIZARDS IDA 5.5 database files.",
    )
    parser.add_argument(
        "--output",
        default=str(repo_root / "project_config.json"),
        help="Project config JSON to update.",
    )
    parser.add_argument(
        "--script",
        default=str(repo_root / "ida55_db_timestamp_config.py"),
        help="Timestamp snapshot script to run.",
    )
    parser.add_argument(
        "--working-dir",
        default=str(repo_root),
        help="Working directory to set before invoking the timestamp script.",
    )
    parser.add_argument(
        "--user",
        help="Optional user account to run the task under. Defaults to the current account.",
    )
    parser.add_argument(
        "--password",
        help="Optional password for --user. Required by schtasks for some accounts.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print the schtasks command without creating or updating the task.",
    )
    return parser.parse_args()


def build_task_command(args: argparse.Namespace) -> str:
    python_path = Path(args.python)
    script_path = Path(args.script)
    working_dir = Path(args.working_dir)
    output_path = Path(args.output)

    inner_command = subprocess.list2cmdline(
        [
            str(python_path),
            str(script_path),
            "--ida-dir",
            args.ida_dir,
            "--output",
            str(output_path),
        ]
    )

    powershell_command = (
        f'Set-Location -LiteralPath \'{str(working_dir).replace("\'", "''")}\'; '
        f"& {inner_command}"
    )

    return subprocess.list2cmdline(
        [
            "powershell.exe",
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-Command",
            powershell_command,
        ]
    )


def validate_time(value: str) -> str:
    parts = value.split(":")
    if len(parts) != 2:
        raise ValueError("time must be in HH:MM format")
    hour = int(parts[0])
    minute = int(parts[1])
    if not (0 <= hour <= 23 and 0 <= minute <= 59):
        raise ValueError("time must be a valid 24-hour HH:MM value")
    return f"{hour:02d}:{minute:02d}"


def main() -> int:
    args = parse_args()
    start_time = validate_time(args.time)
    task_command = build_task_command(args)

    schtasks_command = [
        "schtasks",
        "/Create",
        "/F",
        "/SC",
        "DAILY",
        "/TN",
        args.task_name,
        "/TR",
        task_command,
        "/ST",
        start_time,
    ]

    if args.user:
        schtasks_command.extend(["/RU", args.user])
        if args.password is not None:
            schtasks_command.extend(["/RP", args.password])

    if args.dry_run:
        print(subprocess.list2cmdline(schtasks_command))
        return 0

    subprocess.run(schtasks_command, check=True)
    print(f"Scheduled task configured: {args.task_name}")
    print(f"Runs daily at {start_time}")
    print(f"Task action: {task_command}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
