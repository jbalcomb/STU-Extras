from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

import run_ida55_dump_and_split as dump_and_split


class RunIda55DumpAndSplitTests(unittest.TestCase):
    def test_clear_split_asm_files_removes_only_split_asm(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir) / "out"
            split_dir = root / "MAGIC" / "ovr001"
            split_dir.mkdir(parents=True)
            asm_path = split_dir / "Foo.asm"
            c_path = split_dir / "Foo.c"
            top_level_asm = root / "top.asm"
            report_dir = root / "asm-diff-report"
            report_dir.mkdir(parents=True)
            report_asm = report_dir / "not_split.asm"

            asm_path.write_text("asm", encoding="utf-8")
            c_path.write_text("c", encoding="utf-8")
            top_level_asm.write_text("top", encoding="utf-8")
            report_asm.write_text("report", encoding="utf-8")

            removed_count = dump_and_split.clear_split_asm_files(root)

            self.assertEqual(removed_count, 1)
            self.assertFalse(asm_path.exists())
            self.assertTrue(c_path.exists())
            self.assertTrue(top_level_asm.exists())
            self.assertTrue(report_asm.exists())

    def test_split_asm_paths_are_sorted(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir) / "out"
            first = root / "MAGIC" / "ovr001" / "A.asm"
            second = root / "WIZARDS" / "ovr001" / "B.asm"
            second.parent.mkdir(parents=True)
            first.parent.mkdir(parents=True)
            second.write_text("b", encoding="utf-8")
            first.write_text("a", encoding="utf-8")

            self.assertEqual(dump_and_split.split_asm_paths(root), [first, second])

    def test_ida_export_commands_copy_into_in_with_a_timeout(self) -> None:
        asm_export, function_export = dump_and_split.ida_export_commands("py", 900, dry_run=False)

        self.assertEqual(asm_export[:3], ["py", "-B", str(Path("ida_dasm_export") / "ida55_clean_export.py")])
        self.assertEqual(function_export[:3], ["py", "-B", str(Path("ida_proc_data_export") / "ida55_export_function_data.py")])
        for command in (asm_export, function_export):
            self.assertIn("--copy", command)
            self.assertEqual(command[command.index("--timeout-seconds") + 1], "900")
            self.assertNotIn("--dry-run", command)

    def test_ida_export_commands_pass_dry_run_through(self) -> None:
        for command in dump_and_split.ida_export_commands("py", 900, dry_run=True):
            self.assertIn("--dry-run", command)


if __name__ == "__main__":
    unittest.main()
