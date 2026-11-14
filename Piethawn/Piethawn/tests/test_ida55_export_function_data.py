from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path

from ida_proc_data_export import ida55_export_function_data as export_function_data


class FunctionDataExportTests(unittest.TestCase):
    def test_expected_dump_for_idb_appends_dump_suffix(self) -> None:
        result = export_function_data.expected_dump_for_idb(Path("/ida/MAGIC.idb"))
        self.assertEqual(result, Path("/ida/MAGIC.idb.ida55-functions.txt"))

    def test_json_output_path_targets_out_dir(self) -> None:
        result = export_function_data.json_output_path(Path("/repo/in"), Path("/ida/WIZARDS.idb"))
        self.assertEqual(result, Path("/repo/in/WIZARDS.idb.ida55-functions.json"))

    def test_work_dir_for_job_is_per_game_and_run(self) -> None:
        result = export_function_data.work_dir_for_job(Path("/repo/out/ida55-work"), "MAGIC", "20261111200000")
        self.assertEqual(result, Path("/repo/out/ida55-work/MAGIC-20261111200000"))

    def test_build_ida_command_passes_work_dir_before_separator(self) -> None:
        command = export_function_data.build_ida_command(
            sys.executable,
            Path("idaw.py"),
            Path("script.idc"),
            Path("MAGIC.idb"),
            ida_exe=Path("idaw.exe"),
            work_dir=Path("work"),
        )
        self.assertEqual(
            command,
            [sys.executable, "-B", "idaw.py", "--ida-exe", "idaw.exe", "--work-dir", "work", "--", "-A", "-Sscript.idc", "MAGIC.idb"],
        )

    def test_write_dump_json_records_real_idb_and_work_copy(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            dump = Path(temp_dir) / "work" / "MAGIC.idb.ida55-functions.txt"
            dump.parent.mkdir()
            lines = [
                "idb_path=" + str(Path(temp_dir) / "work" / "MAGIC.idb"),
                "function[0].name=Mark_Time",
                "function[0].segment=seg002",
                "function[0].start_ea=0000",
                "function[0].end_ea=0010",
                "function[0].size=16",
                "function_count=1",
            ]
            dump.write_text("\n".join(lines) + "\n", encoding="utf-8")
            json_path = Path(temp_dir) / "in" / "MAGIC.idb.ida55-functions.json"
            json_path.parent.mkdir()
            export_function_data.write_dump_json(dump, json_path, Path("C:/ida/MAGIC.idb"))

            payload = json.loads(json_path.read_text(encoding="utf-8"))
            self.assertEqual(payload["meta"]["idb_path"], export_function_data.denormalize_for_display(Path("C:/ida/MAGIC.idb")))
            self.assertEqual(payload["meta"]["work_idb_path"], str(Path(temp_dir) / "work" / "MAGIC.idb"))
            self.assertEqual(payload["functions"][0]["name"], "Mark_Time")
            self.assertEqual(sorted(p.name for p in json_path.parent.iterdir()), ["MAGIC.idb.ida55-functions.json"])

    def test_build_ida_command_uses_shared_wrapper(self) -> None:
        command = export_function_data.build_ida_command(
            sys.executable,
            Path("IDA55-Py") / "ida55py" / "idaw.py",
            Path("ida55_dump_functions.idc"),
            Path("MAGIC.idb"),
            Path("MAGIC.ida.log"),
        )
        self.assertEqual(
            command,
            [
                sys.executable,
                "-B",
                str(Path("IDA55-Py") / "ida55py" / "idaw.py"),
                "--",
                "-A",
                "-LMAGIC.ida.log",
                "-Sida55_dump_functions.idc",
                "MAGIC.idb",
            ],
        )


    def test_build_ida_command_passes_ida_exe_and_backup_dir_before_separator(self) -> None:
        command = export_function_data.build_ida_command(
            sys.executable,
            Path("idaw.py"),
            Path("script.idc"),
            Path("MAGIC.idb"),
            ida_exe=Path("idaw.exe"),
            backup_dir=Path("backups"),
        )
        self.assertEqual(
            command,
            [sys.executable, "-B", "idaw.py", "--ida-exe", "idaw.exe", "--backup-dir", "backups", "--", "-A", "-Sscript.idc", "MAGIC.idb"],
        )
    def test_resolve_idaw_wrapper_accepts_shared_wrapper(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir) / "Piethawn" / "Piethawn"
            repo_root.mkdir(parents=True)
            wrapper = Path(temp_dir) / "IDA55-Py" / "ida55py" / "idaw.py"
            wrapper.parent.mkdir(parents=True)
            wrapper.write_text("# wrapper", encoding="utf-8")

            self.assertEqual(export_function_data.resolve_idaw_wrapper(repo_root), wrapper.resolve())

    def test_resolve_idaw_wrapper_rejects_missing_wrapper_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            with self.assertRaisesRegex(FileNotFoundError, "IDAW wrapper not found"):
                export_function_data.resolve_idaw_wrapper(Path(temp_dir))

    def test_verify_file_rejects_empty_file_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "MAGIC.idb.ida55-functions.txt"
            path.write_text("", encoding="utf-8")
            with self.assertRaisesRegex(RuntimeError, "is empty"):
                export_function_data.verify_file(path, "dump")


if __name__ == "__main__":
    unittest.main()
