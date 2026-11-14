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
            Path("IDA55-Py") / "ida55py" / "idaw.py",
            Path("ida55_clean_export.idc"),
            Path("MAGIC.idb"),
        )
        self.assertEqual(command, [sys.executable, "-B", str(Path("IDA55-Py") / "ida55py" / "idaw.py"), "--", "-A", "-Sida55_clean_export.idc", "MAGIC.idb"])

    def test_build_ida_command_with_log(self) -> None:
        command = clean_export.build_ida_command(
            sys.executable,
            Path("IDA55-Py") / "ida55py" / "idaw.py",
            Path("ida55_clean_export.idc"),
            Path("MAGIC.idb"),
            Path("MAGIC.ida.log"),
        )
        self.assertEqual(
            command,
            [sys.executable, "-B", str(Path("IDA55-Py") / "ida55py" / "idaw.py"), "--", "-A", "-LMAGIC.ida.log", "-Sida55_clean_export.idc", "MAGIC.idb"],
        )


    def test_build_ida_command_passes_ida_exe_and_backup_dir_before_separator(self) -> None:
        command = clean_export.build_ida_command(
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
    def test_resolve_idaw_wrapper_accepts_repo_wrapper(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir) / "Piethawn" / "Piethawn"
            repo_root.mkdir(parents=True)
            wrapper = Path(temp_dir) / "IDA55-Py" / "ida55py" / "idaw.py"
            wrapper.parent.mkdir(parents=True)
            wrapper.write_text("# wrapper", encoding="utf-8")

            self.assertEqual(clean_export.resolve_idaw_wrapper(repo_root), wrapper.resolve())

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


class CleanExportWorkCopyTests(unittest.TestCase):
    """The clean export runs IDA on a work copy and copies its outputs into in/. Powered by Claude."""

    def test_work_dir_for_job_is_per_game_and_run(self) -> None:
        result = clean_export.work_dir_for_job(Path("/repo/out/ida55-work"), "MAGIC", "20261111200000")
        self.assertEqual(result, Path("/repo/out/ida55-work/MAGIC-clean-20261111200000"))

    def test_build_ida_command_passes_work_dir_before_separator(self) -> None:
        command = clean_export.build_ida_command(
            sys.executable, Path("idaw.py"), Path("x.idc"), Path("MAGIC.idb"), ida_exe=Path("idaw.exe"), work_dir=Path("work")
        )
        self.assertEqual(
            command,
            [sys.executable, "-B", "idaw.py", "--ida-exe", "idaw.exe", "--work-dir", "work", "--", "-A", "-Sx.idc", "MAGIC.idb"],
        )

    def test_run_exports_copies_outputs_removes_work_and_leaves_real_idb_alone(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            idb_dir = root / "ida"
            idb_dir.mkdir()
            real_idb = idb_dir / "MAGIC.idb"
            real_idb.write_bytes(b"real-idb")
            work_root, out_dir, log_dir = root / "work", root / "in", root / "logs"
            stamp = "20261111200000"
            job = clean_export.ExportJob(game="MAGIC", idb_path=real_idb)
            work_idb = clean_export.work_idb_for_job(work_root, job, stamp)
            seen = {}

            def fake_run_command(command, cwd, dry_run, transcript_path=None, timeout_seconds=None, startup_enter=False):
                """Stand in for idaw.py + IDA: write the IDC's outputs next to the work copy. Powered by Claude."""
                seen["command"] = list(command)
                work_idb.parent.mkdir(parents=True)
                work_idb.write_bytes(b"copy")
                for suffix in clean_export.OUTPUT_SUFFIXES:
                    work_idb.with_suffix(suffix).write_text(f"MAGIC {suffix}", encoding="utf-8")
                for suffix in clean_export.SETTINGS_DUMP_SUFFIXES:
                    Path(str(work_idb.with_suffix("")) + suffix).write_text("phase=x", encoding="utf-8")
                return 0

            original_run_command = clean_export.run_command
            clean_export.run_command = fake_run_command
            try:
                with redirect_stdout(StringIO()):
                    clean_export.run_exports(
                        sys.executable, Path("idaw.py"), Path("x.idc"), [job], root, False, clean_export.make_summary(),
                        log_dir, False, 0, False, work_root=work_root, stamp=stamp, out_dir=out_dir,
                    )
            finally:
                clean_export.run_command = original_run_command

            self.assertIn("--work-dir", seen["command"])
            self.assertEqual(seen["command"][-1], str(real_idb))
            self.assertEqual(sorted(p.name for p in out_dir.iterdir()), ["MAGIC.asm", "MAGIC.inc", "MAGIC.lst", "MAGIC.map"])
            self.assertEqual((out_dir / "MAGIC.asm").read_text(encoding="utf-8"), "MAGIC .asm")
            self.assertTrue((log_dir / "MAGIC.ida55-clean-export-active.txt").exists())
            self.assertFalse(work_idb.parent.exists())
            self.assertEqual(real_idb.read_bytes(), b"real-idb")
            self.assertEqual(sorted(p.name for p in idb_dir.iterdir()), ["MAGIC.idb"])

    def test_missing_output_keeps_work_folder_and_in_untouched_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            real_idb = root / "MAGIC.idb"
            real_idb.write_bytes(b"real-idb")
            work_root, out_dir = root / "work", root / "in"
            job = clean_export.ExportJob(game="MAGIC", idb_path=real_idb)
            work_idb = clean_export.work_idb_for_job(work_root, job, "s")

            def fake_run_command(command, cwd, dry_run, transcript_path=None, timeout_seconds=None, startup_enter=False):
                work_idb.parent.mkdir(parents=True)
                work_idb.with_suffix(".asm").write_text("only asm", encoding="utf-8")
                return 0

            original_run_command = clean_export.run_command
            clean_export.run_command = fake_run_command
            try:
                with redirect_stdout(StringIO()), self.assertRaisesRegex(RuntimeError, "Missing files"):
                    clean_export.run_exports(
                        sys.executable, Path("idaw.py"), Path("x.idc"), [job], root, False, clean_export.make_summary(),
                        root / "logs", False, 0, False, work_root=work_root, stamp="s", out_dir=out_dir,
                    )
            finally:
                clean_export.run_command = original_run_command

            self.assertTrue(work_idb.parent.exists())
            self.assertEqual(list(out_dir.iterdir()), [])


if __name__ == "__main__":
    unittest.main()
