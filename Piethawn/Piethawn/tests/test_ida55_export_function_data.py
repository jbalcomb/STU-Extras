from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

from ida_proc_data_export import ida55_export_function_data as export_function_data


class FunctionDataExportTests(unittest.TestCase):
    def test_expected_dump_for_idb_appends_dump_suffix(self) -> None:
        result = export_function_data.expected_dump_for_idb(Path("/ida/MAGIC.idb"))
        self.assertEqual(result, Path("/ida/MAGIC.idb.ida55-functions.txt"))

    def test_repo_dump_path_targets_in_directory(self) -> None:
        result = export_function_data.repo_dump_path(Path("/repo"), Path("/ida/WIZARDS.idb"))
        self.assertEqual(result, Path("/repo/in/WIZARDS.idb.ida55-functions.txt"))

    def test_build_ida_command_uses_shared_wrapper(self) -> None:
        command = export_function_data.build_ida_command(
            sys.executable,
            Path("ida_automation") / "idaw.py",
            Path("ida55_dump_functions.idc"),
            Path("MAGIC.idb"),
            Path("MAGIC.ida.log"),
        )
        self.assertEqual(
            command,
            [
                sys.executable,
                "-B",
                str(Path("ida_automation") / "idaw.py"),
                "--",
                "-A",
                "-LMAGIC.ida.log",
                "-Sida55_dump_functions.idc",
                "MAGIC.idb",
            ],
        )

    def test_resolve_idaw_wrapper_accepts_shared_wrapper(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            wrapper = repo_root / "ida_automation" / "idaw.py"
            wrapper.parent.mkdir()
            wrapper.write_text("# wrapper", encoding="utf-8")

            self.assertEqual(export_function_data.resolve_idaw_wrapper(repo_root), wrapper)

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
