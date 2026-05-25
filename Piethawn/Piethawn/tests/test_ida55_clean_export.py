from __future__ import annotations

import os
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from io import StringIO
from pathlib import Path

from ida_dasm_export import ida55_clean_export as clean_export


class CleanExportTests(unittest.TestCase):
    def test_normalize_windows_path_under_posix(self) -> None:
        repo_root = Path("/repo")
        result = clean_export.normalize_path(r"C:\STU_DASM\IDA55_MOM131", repo_root)
        if os.name == "nt":
            self.assertEqual(result, Path(r"C:\STU_DASM\IDA55_MOM131"))
        else:
            self.assertEqual(result, Path("/mnt/c/STU_DASM/IDA55_MOM131"))

    def test_normalize_relative_path(self) -> None:
        repo_root = Path("/repo")
        self.assertEqual(clean_export.normalize_path("in", repo_root), repo_root / "in")

    def test_expected_outputs_for_idb(self) -> None:
        outputs = clean_export.expected_outputs_for_idb(Path("/ida/MAGIC.idb"))
        self.assertEqual(
            [path.name for path in outputs],
            ["MAGIC.map", "MAGIC.asm", "MAGIC.inc", "MAGIC.lst"],
        )

    def test_build_ida_command(self) -> None:
        command = clean_export.build_ida_command(
            sys.executable,
            Path("ida_automation") / "idaw.py",
            Path("ida55_clean_export.idc"),
            Path("MAGIC.idb"),
        )
        self.assertEqual(command, [sys.executable, "-B", str(Path("ida_automation") / "idaw.py"), "--", "-A", "-Sida55_clean_export.idc", "MAGIC.idb"])

    def test_build_ida_command_with_log(self) -> None:
        command = clean_export.build_ida_command(
            sys.executable,
            Path("ida_automation") / "idaw.py",
            Path("ida55_clean_export.idc"),
            Path("MAGIC.idb"),
            Path("MAGIC.ida.log"),
        )
        self.assertEqual(
            command,
            [sys.executable, "-B", str(Path("ida_automation") / "idaw.py"), "--", "-A", "-LMAGIC.ida.log", "-Sida55_clean_export.idc", "MAGIC.idb"],
        )

    def test_resolve_idaw_wrapper_accepts_repo_wrapper(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            wrapper = repo_root / "ida_automation" / "idaw.py"
            wrapper.parent.mkdir()
            wrapper.write_text("# wrapper", encoding="utf-8")

            self.assertEqual(clean_export.resolve_idaw_wrapper(repo_root), wrapper)

    def test_resolve_idaw_wrapper_rejects_missing_wrapper_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            with self.assertRaisesRegex(FileNotFoundError, "IDAW wrapper not found"):
                clean_export.resolve_idaw_wrapper(Path(temp_dir))

    def test_verify_files_accepts_non_empty_files(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "MAGIC.asm"
            path.write_text("content", encoding="utf-8")
            clean_export.verify_files([path])

    def test_verify_files_rejects_missing_file_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "MAGIC.asm"
            with self.assertRaisesRegex(RuntimeError, "Missing files"):
                clean_export.verify_files([path])

    def test_verify_files_rejects_empty_file_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "MAGIC.asm"
            path.write_text("", encoding="utf-8")
            with self.assertRaisesRegex(RuntimeError, "Empty files"):
                clean_export.verify_files([path])

    def test_tail_text_returns_last_lines(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "run.log"
            path.write_text("one\ntwo\nthree\n", encoding="utf-8")
            self.assertEqual(clean_export.tail_text(path, 2), "two\nthree")

    def test_tail_text_missing_file_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "missing.log"
            self.assertEqual(clean_export.tail_text(path), "")

    def test_run_command_writes_process_transcript(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            transcript = Path(temp_dir) / "process.log"
            result = clean_export.run_command(
                [sys.executable, "-c", "print('transcript-ok')"],
                Path(temp_dir),
                dry_run=False,
                transcript_path=transcript,
                timeout_seconds=10,
            )

            self.assertEqual(result, 0)
            text = transcript.read_text(encoding="utf-8")
            self.assertIn("transcript-ok", text)
            self.assertIn("[returncode]\n0", text)

    def test_run_command_timeout_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            transcript = Path(temp_dir) / "process.log"
            with self.assertRaises(TimeoutError):
                clean_export.run_command(
                    [sys.executable, "-c", "import time; time.sleep(3)"],
                    Path(temp_dir),
                    dry_run=False,
                    transcript_path=transcript,
                    timeout_seconds=1,
                )

            text = transcript.read_text(encoding="utf-8")
            self.assertIn("[timeout]", text)

    def test_print_summary_reports_unproven_dry_run_work(self) -> None:
        summary = clean_export.make_summary()
        summary.add_check("IDA executable resolved")
        summary.add_skipped("IDA was not launched")
        summary.add_unproven("Generated files are untested")

        output = StringIO()
        with redirect_stdout(output):
            clean_export.print_summary(summary, dry_run=True)

        text = output.getvalue()
        self.assertIn("Mode: dry-run", text)
        self.assertIn("Worked/checked:", text)
        self.assertIn("Skipped:", text)
        self.assertIn("Not proven by this run:", text)


if __name__ == "__main__":
    unittest.main()
