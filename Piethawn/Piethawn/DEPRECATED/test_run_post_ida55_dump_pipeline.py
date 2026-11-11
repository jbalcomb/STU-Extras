from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

import run_post_ida55_dump_pipeline as pipeline


class RunPostIda55DumpPipelineTests(unittest.TestCase):
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

            removed_count = pipeline.clear_split_asm_files(root)

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

            self.assertEqual(pipeline.split_asm_paths(root), [first, second])


if __name__ == "__main__":
    unittest.main()
